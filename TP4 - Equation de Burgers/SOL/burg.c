#include "burg.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
df_burg null_dfc = {0};

// resolution de l'equation de burgers
// compiler avec gcc burg.c -lm

#define _BURG


int main(void){

  df_burg dfc = null_dfc;

  int nx = 100;
#ifdef _BURG
  double xmin = -1;
  double xmax = 2;
#else
  double xmin = 0;
  double xmax = 1;
#endif
  
  init_df_burg(&dfc, nx, xmin, xmax);

  double tmax = 0.5;

  double cfl = 0.5;
  compute_df(&dfc, tmax, cfl); 

  
  plot_data(&dfc);

  return 0;


}


#ifdef _BURG

double flux(double w){
  double f = w * w / 2;
  return f;
}

double dflux(double w){
  double df = w;
  return df;
}


void u_exact(double x, double t, double *u){

  if (t < 1) {
    if (x < t) {
      *u = 1;
    } else if (x > 1){
      *u = 0;
    } else {
      *u = (1 - x) / (1 - t);
    }
  } else {
    if (x < t / 2 + 0.5) {
      *u = 1;
    } else {
      *u = 0;
    }
  }

}

#else

#define _VMAX 130.
#define _RHO 200.

double flux(double w){ 
  double f = _VMAX * w * (1 - w / _RHO);
  return f;
}

double dflux(double w){
  double df = _VMAX * (1 - 2 * w / _RHO);
  return df;
}


void u_exact(double x, double t, double *u){
  
  *u = _RHO;

}



#endif

double fluxnum(double a, double b){

  double vmax = fabs(dflux(a)) > fabs(dflux(b)) ?
    fabs(dflux(a)) : fabs(dflux(b));

  double f = (flux(a) + flux(b)) / 2
    - vmax / 2 * (b - a);

  return f;

}

void init_df_burg(df_burg *dfc, int nx,
		  double xmin, double xmax){

  dfc->nx = nx;
  dfc->xmin = xmin;
  dfc->xmax = xmax;
  dfc->Lx = xmax - xmin;

  dfc->dx = dfc->Lx / nx;

  dfc->xx = malloc((nx + 2) * sizeof(double));
  
  dfc->un = malloc((nx + 2) * sizeof(double));
  dfc->unp1 = malloc((nx + 2) * sizeof(double));

  for(int i = 0; i < nx + 2; i++){
    dfc->xx[i] = xmin + (i - 0.5) * dfc->dx;
    double t = 0;
    u_exact(dfc->xx[i], t, dfc->un + i);
    dfc->unp1[i] = dfc->un[i];
  }
  

}

void plot_data(df_burg *dfc){

  FILE *plotfile;

  plotfile = fopen("plot.dat", "w");

  double err_sup = 0;
  double err_L2 = 0;
  
  for(int i = 0; i < dfc->nx + 2; i++){
    double u;
    u_exact(dfc->xx[i], dfc->tnow, &u);
    fprintf(plotfile, "%f %f %f\n", dfc->xx[i], dfc->un[i], u);
    double errloc = fabs(u - dfc->un[i]);
    err_L2 += dfc->dx * errloc * errloc;
    err_sup = errloc > err_sup ? errloc : err_sup;
  }

  err_L2 = sqrt(err_L2);

  printf("erreur L2=%f L_infini=%f \n",
	 err_L2, err_sup);
  
  fclose(plotfile);

  int status =
    system("gnuplot plotcom 2> /dev/null");
  assert(status == 0);
  
}



void compute_df(df_burg *dfc, double tmax, double cfl){

  double dx = dfc->dx;
  double dt;
  dfc->cfl = cfl;
  dfc->dt = dt;
  
  int nx = dfc->nx;

  dfc->tmax = tmax;
  dfc->tnow = 0;


  while(dfc->tnow < tmax){

    
    double vmax = 0;
    for(int i = 0; i < nx + 2; i++){
      vmax = vmax > fabs(dflux(dfc->un[i])) ?
	vmax : fabs(dflux(dfc->un[i]));
    }

    dfc->dt = cfl * dx / vmax;
    dt = dfc->dt;

    for(int i = 1; i < nx + 1; i++){

      dfc->unp1[i] = dfc->un[i];
      double flux;
      flux = fluxnum(dfc->un[i], dfc->un[i + 1]);
      dfc->unp1[i] -= dt / dx * flux;
      flux = fluxnum(dfc->un[i - 1], dfc->un[i]);
      dfc->unp1[i] += dt / dx * flux;
      
    }
    
    for(int i = 1; i < nx + 1; i++)
      dfc->un[i] = dfc->unp1[i];

    dfc->tnow += dt;
    printf("tnow =%f dt=%f vmax=%f\n",
	   dfc->tnow, dt, vmax);

    u_exact(dfc->xx[0], dfc->tnow, dfc->un + 0);
    u_exact(dfc->xx[nx + 1], dfc->tnow, dfc->un + nx + 1);

  }

}



