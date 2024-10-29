#include <stdio.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_fft_real.h>

int main()
{
   

    double data[] = {1.0, 2.0, 3.0, 2.0, 1.0, 1.0, 2.0, 3.0};
    size_t stride = 1;
    size_t n = 8;
    int x = gsl_fft_real_radix2_transform(data, stride, n);
    printf("gsl_fft_real_radix2_transform(data, stride, n) = %d\n", x);
    for (int i=0;i<8;i++){
        printf("i[%d]:%f\n",i, data[i]);
    }
    return 0;
}

// THIS COMPILED:
// gcc -I/opt/homebrew/Cellar/gsl/2.8/include -L/opt/homebrew/Cellar/gsl/2.8/lib -lgsl -lgslcblas test.c    

