#include <stdio.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <math.h>

#define REAL 0
#define IMAG 1

const float amp = 100;
const int period = 64; // must be power of 2
const int repeats_exp = 0; 
const int total = period << repeats_exp; //total = period * (2 ^ repeats_exp)

void square_wave(double data[]);
void sine_wave(double data[]);
void print_data(double data[], int include_zeros);
void print_data_complex(gsl_complex_packed_array data, int include_zeros);

int main()
{
    size_t stride = 1;
    double data[total];
    printf("square:\n");
    square_wave(data);
    print_data(data, 1);
    int x = gsl_fft_real_radix2_transform(data, stride, total);
    double temp[total*2]; 
    gsl_complex_packed_array complex_coeff = temp;
    gsl_fft_halfcomplex_radix2_unpack(data, complex_coeff, 1, total);
    print_data_complex(complex_coeff, 0);
    printf("sine:\n");
    sine_wave(data);
    // print_data(data,0);
    x = gsl_fft_real_radix2_transform(data, stride, total);
    gsl_fft_halfcomplex_radix2_unpack(data, complex_coeff, 1, total);
    print_data_complex(complex_coeff,0);
    return 0;
}

void square_wave(double data[])
{
    double amp = 0.5;
    for (int i = 0; i < total; i++)
    {
        if ((i+period/4) % (period / 2) == 0)
        {
            amp = -amp;
        }
        data[i] = (double)amp;
    }
}
void sine_wave(double data[]){
    for (int i=0;i<total;i++){
        data[i] = amp * sin((((double)i)/period) * M_PI * 2);
    }
}
void print_data(double data[], int include_zeros){
    const double threshold = 0.001;
    for (int i = 0; i < total; i++)
    {
        if (!include_zeros && data[i]==0){
            continue;
        }
        if (!include_zeros && fabs(data[i]) < threshold){
            continue;
        }
        printf("i[%d]:%f\n", i, data[i]);
    }
}
void print_data_complex(gsl_complex_packed_array data, int include_zeros){
    const double threshold = 0.001;
    printf("complex coeffs:\n");
    for (int i = 0; i < total*2; i+=2)
    {
        double real = data[i+REAL];
        double imag = data[i+IMAG];

        if (!include_zeros && fabs(real)<threshold && fabs(imag)<threshold){
            continue;
        }

        printf("i[%d]:%f + i%f (amp = %f)\n", i/2, real, imag, sqrt(real*real + imag*imag));
    }
}
// THIS COMPILED:
// gcc -I/opt/homebrew/Cellar/gsl/2.8/include -L/opt/homebrew/Cellar/gsl/2.8/lib -lgsl -lgslcblas test.c
