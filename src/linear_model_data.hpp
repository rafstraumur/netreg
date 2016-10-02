/**
 * Author: Simon Dirmeier
 * Email: netreg@simon-dirmeier.net
 */

#ifndef NETREG_LINEARMODELDATA_HPP
#define NETREG_LINEARMODELDATA_HPP

#include <vector>
#include <string>
#include <utility>

#ifndef ARMA_DONT_USE_WRAPPER
#define ARMA_DONT_USE_WRAPPER
#endif
#include <armadillo>

#include "family.hpp"
#include "types.hpp"

namespace netreg
{
    /**
     * Data-structure for all required data for a linear model
     */
    class linear_model_data
    {
    protected:
        /**
         * Protected constructor in order to avoid instantiation.
         *
         * @param x the design matrix as pointer
         * @param y the desponse matrix as pointer
         * @param n number of samples
         * @param p number of covariables
         * @param q number of responses
         * @param niter maximal number of iterations
         * @param thresh the convergence threshold
         * @param fam of distribution of y
         */
        linear_model_data(double *x, double *y,
                          const int n, const int p, const int q,
                          const int niter, const double thresh,
                          const family fam)
            : N(n), P(p), Q(q),
              X(x, n, p),
              Y(y, n, q),
              intrcpt(q), coeffs(p, q, arma::fill::ones),
              THRESH(thresh), N_ITER(niter),
              TXX(p, p), TXY(p, q),
              family_(fam)
        {
            matrix<double> TX = X.t();
            TXX = TX * X;
            TXY = TX * Y;
        }

    public:

        /**
         * Getter for the family of the distribution of the response.
         *
         * @return returns the family
         */
        const family family()
        {
            return family_;
        }

        /**
         * Getter for number of samples.
         *
         * @return returns the number of samples
         */
        const int sample_count()
        {
            return N;
        }

        /**
         * Getter for number of responses.
         *
         * @return returns the number of responses
         */
        const int response_count()
        {
            return Q;
        }

        /**
         * Getter for number of covariables.
         *
         * @return returns the number of covariables
         */
        const int covariable_count()
        {
            return P;
        }

        /**
         * Getter for the coefficient matrix at element i,i
         *
         * @param i the index of the row
         * @param j the index of the column
         * @return a reference to an element in the coefficient matrix
         */
        double &coefficients(const int i, const int j) 
        {
            return coeffs(i, j);
        }

        /**
         * Getter for the intercept vector.
         *
         * @return a reference to the vector of intercepts
         */
        cvector<double> &intercept() 
        {
            return intrcpt;
        }

        /**
         * Getter for the design matrix.
         *
         * @return a reference to the design matrix
         */
         matrix<double> &design()
        {
            return X;
        }

        /**
         * Getter for the response matrix.
         *
         * @return a reference to the response matrix
         */
         matrix<double> &response()
        {
            return Y;
        }

        /**
         * Getter for the coefficient matrix.
         *
         * @return a reference to the coefficient matrix
         */
        matrix<double> &coefficients() 
        {
            return coeffs;
        }

        /**
         * Getter for X'X matrix.
         *
         * @return a reference to X'X matrix.
         */
        matrix<double> &txx() 
        {
            return TXX;
        }

        /**
         * Getter for X'Y matrix.
         *
         * @return a reference to X'Y matrix.
         */
        matrix<double> &txy() 
        {
            return TXY;
        }

        /**
         * Getter for the number of maximal iterations.
         *
         * @return the max number of iterations
         */
        const int max_iter() 
        {
            return N_ITER;
        }

        /**
         * Getter for the convergence threshold of CCD.
         *
         * @return the convergence threshold.
         */
        double threshold()
        {
            return THRESH;
        }

    protected:
        const int N;             // number of samples: n
        const int P;             // number of covariables: p
        const int Q;             // number of responses: q
        matrix<double> X;        // (n x p)-dimensional design matrix
        matrix<double> Y;        // (n x q)-dimensional response matrix
        cvector<double> intrcpt; // q-dimensional intercept vector
        matrix<double> coeffs;   // (p x q)-dimensional coefficient matrix
        const double THRESH;     // convergence threshold
        const int N_ITER;        // max number iterations if CCD does not converge
        matrix<double> TXX;      // (p x p)-dimensional matrix: X'X
        matrix<double> TXY;      // (p x q)-dimensional matrix: X'Y
        enum family family_;      // family of distribution of y
    };
}
#endif //NETREG_LINEARMODELDATA_HPP
