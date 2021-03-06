# Generated by using Rcpp::compileAttributes() -> do not edit by hand
# Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

node_degrees_ <- function(x) {
    .Call(`_netReg_node_degrees_`, x)
}

laplacian_ <- function(x) {
    .Call(`_netReg_laplacian_`, x)
}

intercept_ <- function(X, Y, B) {
    .Call(`_netReg_intercept_`, X, Y, B)
}

# Register entry points for exported C++ functions
methods::setLoadAction(function(ns) {
    .Call('_netReg_RcppExport_registerCCallable', PACKAGE = 'netReg')
})
