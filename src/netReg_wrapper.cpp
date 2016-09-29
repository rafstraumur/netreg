/**
 * Author: Simon Dirmeier
 * Email: netreg@simon-dirmeier.net
 */

#include <R.h>
#include <Rinternals.h>
#include <Rmath.h>

#include "edgenet_wrapper.hpp"

/**
 * Extern C++ binding for R! (name mangling)
 */

double *B_;
double *mu_;
int *foldid_;
double lamb_;
double psi_gx_;
double psi_gy_;

extern "C"
{

/**
 * Implementation of Edgenet, a edge-based regularized regression model.
 *
 * @param XS the (ns x ps)-dimensional design matrix
 * @param YS the (ns x qs)-dimensional response matrix
 * @param GXS the (ps x ps)-prior graph for XS
 * @param GYS the (qs x qs)-prior graph for YS
 * @param ns number of observations/samples/rows
 * @param ps number of covariates
 * @param qs number of responses
 * @param lamdbass penalization value for LASSO
 * @param psi_gxs weighting value of GX
 * @param psi_gys weighting value of GY
 * @param niters max number of iterations if parameter estimation does not converge in time
 * @param threshs convergence threshold
 */
SEXP edgenet_(SEXP XS, SEXP YS,
              SEXP GXS, SEXP GYS,
              SEXP ns, SEXP ps, SEXP qs,
              SEXP lambdass,
              SEXP psi_gxs, SEXP psi_gys,
              SEXP niters, SEXP threshs,
              SEXP familys)
{
    // get number of samples
    const int N = (*INTEGER(ns));
    // get number of covariables
    const int P = (*INTEGER(ps));
    // get number of responses
    const int Q = (*INTEGER(qs));
    // cast R design matrix to pointer to double
    double *X = REAL(XS);
    // cast R response matrix to pointer to double
    double *Y = REAL(YS);
    // cast R prior matrix for X to pointer to double
    double *GX = REAL(GXS);
    // cast R prior matrix for Y to pointer to double
    double *GY = REAL(GYS);
    // cast R lambda values to pointer to double
    const double lambda = (*REAL(lambdass));
    // cast R weighting values for GX to pointer to double
    const double psigx = (*REAL(psi_gxs));
    // cast R weighting values for GY to pointer to double
    const double psigy = (*REAL(psi_gys));
    // get number of max iterations
    const int niter = (*INTEGER(niters));
    // get convergence threshold
    const double thresh = (*REAL(threshs));
    // get family
    const char *family = CHAR(STRING_ELT(familys, 0));
    // call wrapper
    do_edgenet_(X, Y, GX, GY,
                N, P, Q,
                lambda, psigx, psigy,
                niter, thresh, family);
    // protection counter that is needed for not activating garbage collection
    int prtCnt = 0;
    // R object for coefficient matrix
    SEXP BS = PROTECT(allocMatrix(REALSXP, P, Q));
    // protect from gc
    prtCnt++;
    // R object for intercept vector
    SEXP intercept = PROTECT(allocVector(REALSXP, Q));
    // protect from gc
    prtCnt++;
    double *B = REAL(BS);
    double *b0 = REAL(intercept);
    for (int i = 0; i < Q; ++i)
    {
        // safe intercepts for R vector
        b0[i] = mu_[i];
        for (int j = 0; j < P; ++j)
        {
            // safe coefficients for R matrix
            B[j + P * i] = B_[j + P * i];
        }
    }
    // create a R list of size 2 that can be returned
    SEXP OS = PROTECT(allocVector(VECSXP, 2));
    prtCnt++;
    // set first element of list to the coef matrix
    SET_VECTOR_ELT(OS, 0, BS);
    // set second element of list to intercept vector
    SET_VECTOR_ELT(OS, 1, intercept);
    // create name array
    SEXP nms = PROTECT(allocVector(STRSXP, 2));
    prtCnt++;
    SET_STRING_ELT(nms, 0, mkChar("coefficients"));
    SET_STRING_ELT(nms, 1, mkChar("intercept"));
    // assign names to list
    setAttrib(OS, R_NamesSymbol, nms);
    // release SEXPs for garbage collection
    UNPROTECT(prtCnt);
    // return results to R
    return OS;
}

/**
 * Implementation of cross-validation for Edgenet. Finds and returns the optimal shrinkage values given a specific data-set.
 *
 * @param XS the (ns x ps)-dimensional design matrix
 * @param YS the (ns x qs)-dimensional response matrix
 * @param GXS the (ps x ps)-prior graph for XS
 * @param GYS the (qs x qs)-prior graph for YS
 * @param ns number of observations/samples/rows
 * @param ps number of covariates
 * @param qs number of responses
 * @param psi_gxs weighting value of GX
 * @param psi_gys weighting value of GY
 * @param niters max number of iterations if parameter estimation does not converge in time
 * @param threshs convergence threshold
 * @param nfolds the number of cross-validation sets created (as in k-fold cv)
 * @param foldids integer vector of assignments of observations to folds (i.e. vector of ns elements,  \in {1, ..., nfolds}
 * @param lenfoldids length of the vector above
 */
SEXP cv_edgenet_(SEXP XS, SEXP YS, SEXP GXS, SEXP GYS,
                 SEXP ns, SEXP ps, SEXP qs,
                 SEXP psi_gxs, SEXP psi_gys,
                 SEXP niters, SEXP threshs,
                 SEXP nfolds, SEXP foldids, SEXP lenfoldids,
                 SEXP familys)
{
    double *X = REAL(XS);
    // cast R response matrix to pointer to double
    double *Y = REAL(YS);
    // cast R prior matrix for X to pointer to double
    double *GX = REAL(GXS);
    // cast R prior matrix for Y to pointer to double
    double *GY = REAL(GYS);
    // get number of samples
    const int N = (*INTEGER(ns));
    // get number of covariables
    const int P = (*INTEGER(ps));
    // get number of responses
    const int Q = (*INTEGER(qs));
    // cast R design matrix to pointer to double
    // cast R weighting values for GX to pointer to double
    const double psigx = (*REAL(psi_gxs));
    // cast R weighting values for GY to pointer to double
    const double psigy = (*REAL(psi_gys));
    // get number of max iterations
    const int niter = (*INTEGER(niters));
    // get convergence threshold
    const double thresh = (*REAL(threshs));
    // the number of folds
    const int nfold = (*INTEGER(nfolds));
    // the fold assignments (usually not given)
    int *fold_ids = (INTEGER(foldids));
    // the length of the fold_ids array
    const int foldid_len = (*INTEGER(lenfoldids));
    // get family
    const char *family = CHAR(STRING_ELT(familys, 0));
    // call wrapper
    do_cv_edgenet_(X, Y, GX, GY,
                   N, P, Q, psigx, psigy, niter, thresh,
                   nfold, fold_ids, foldid_len, family);
    // protection counter that is needed for not activating garbage collection
    int prtCnt = 0;
    SEXP shrink = PROTECT(allocVector(REALSXP, 3));
    prtCnt++;
    REAL(shrink)[0] = lamb_;
    REAL(shrink)[1] = psi_gx_;
    REAL(shrink)[2] = psi_gy_;
    // fold ids
    SEXP folds = PROTECT(allocVector(INTSXP, N));
    prtCnt++;
    for (int i = 0; i < N; ++i) INTEGER(folds)[i] = foldid_[i];
    delete[] foldid_;
    // create a R list of size 2 that can be returned
    SEXP OS = PROTECT(allocVector(VECSXP, 2));
    prtCnt++;
    // set first element of list to the coef matrix
    SET_VECTOR_ELT(OS, 0, shrink);
    // set second element of list to intercept vector
    SET_VECTOR_ELT(OS, 1, folds);
    // create name array
    SEXP nms = PROTECT(allocVector(STRSXP, 2));
    prtCnt++;
    SET_STRING_ELT(nms, 0, mkChar("shrinkage_parameters"));
    SET_STRING_ELT(nms, 1, mkChar("fold_ids"));
    // assign names to list
    setAttrib(OS, R_NamesSymbol, nms);
    // release SEXPs for garbage collection
    UNPROTECT(prtCnt);
    // return results to R
    return OS;
}

};