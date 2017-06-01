set table "scheme_normal-function_1.pgf-plot.table"; set format "%.5f"
set samples 100.0; set parametric; plot [t=0.0:1.0] [] [] t+(2.0*t-1.0)/sqrt(2.0*(1.0+2.0*t*(t-1.0))),1.0/sqrt(2.0*(1.0+2.0*t*(t-1.0)))
