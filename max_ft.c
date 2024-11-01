#include <stdio.h>
#include <math.h>

#define REAL 0
#define IMAG 1

const int period = 64; // must be power of 2
const int repeats_exp = 0; 
// const int total = period << repeats_exp; //total = period * (2 ^ repeats_exp)

void square_wave(double data[]);
// void sine_wave(double data[], int total);
void term(double sk, double n, double P, double T, int t, double out[]);



//return magnitude of term t in sum for harmonic n
// sk is function sample, P is total sample length, n is harmonic number, T is delta time for sampling, t is time
void term(double sk, double n, double P, double T, int t, double out[]){
    double c1 = T * sk;
    double inner = -2 * M_PI * n/P * t;
    double re = cos(inner);
    double im = sin(inner);
    out[REAL] = c1 * re;
    out[IMAG] = c1 * im;
    return;
}
double harm(double data[], double n, double P, double T, double out[]){
    int total = P/T;
    double sum[2] = {0,0};
    
    for (int t=0;t<total;t++){
        double newterm[2];
        term(data[t],n,P,T,t,newterm);
        sum[0] += newterm[0];
        sum[1] += newterm[1];
        // printf("debug:harm term add %f + %f*i | running total %f + %f *i\n",newterm[REAL], newterm[IMAG], sum[REAL], sum[IMAG]);
    }
    out[0] = (T/P) * sum[0];
    out[1] = (T/P) * sum[1];
    return sqrt(out[REAL] * out[REAL] + out[IMAG] * out[IMAG]);
}

void print_data(double data[], int total, int include_zeros){
    for (int i=0; i<total; i++){
        if (data[i] == 0 && !include_zeros){
            continue;
        }
        printf("data[%d]:%f\n",i,data[i]);
    }
}

// void square_wave(double data[])
// {
//     double amp = 0.5;
//     for (int i = 0; i < total; i++)
//     {
//         if ((i+period/4) % (period / 2) == 0)
//         {
//             amp = -amp;
//         }
//         data[i] = (double)amp;
//     }
// }
void sine_wave(double data[], double amp, int total){
    for (int i=0;i<total;i++){
        data[i] = amp * sin((((double)i)/total) * M_PI * 2);
    }
}
// // THIS COMPILED:
// // gcc -I/opt/homebrew/Cellar/gsl/2.8/include -L/opt/homebrew/Cellar/gsl/2.8/lib -lgsl -lgslcblas test.c
void test(){
    const int total = 100;
    double samples[total];
    const float amp = 100;
    sine_wave(samples,amp,100);
    // print_data(samples,total, 1);   
    int t = 3;
    double phase[2];
    // term(samples[t], 1, 100, 1, t, phase);
    // printf("term: %f + %f * i\n",phase[REAL], phase[IMAG]);
    // harm(samples, 1, 100, 1, phase);
    // printf("harm: %f + %f * i\n",phase[REAL], phase[IMAG]);

    
    double harmExpsRealArr[total];
    double harmExpsImagArr[total];
    double *harmExpsRealp = &harmExpsRealArr[total/2];
    double *harmExpsImagp = &harmExpsImagArr[total/2];
    for(int n=-total/2;n<total/2;n++){
        double mag  = harm(samples,n,total,1,phase);
        if (mag < 0.001){
            continue;
        }
        // printf("harm %d: %f + %f*i\n",n, phase[REAL], phase[IMAG]);
        harmExpsRealp[n] = phase[REAL];
        harmExpsImagp[n] = phase[IMAG];
    }
    //calculate harmonics in form fn(x) = A0 + Sum[An cos(2pi * n/p) + Bn sin(2pi * n/p)]
    double A0 = harmExpsRealp[0];
    for (int n=0;n<total/2;n++){
        double An = harmExpsRealp[n] + harmExpsRealp[-n] ;
        double Bn = -(harmExpsImagp[n] - harmExpsImagp[-n]);
        if (A0==0 && An==0 && Bn==0){
            continue;
        }
        printf("harmonic %d: %f + %f*Cos(2pi * %d/%d * t) + %f*Sin(2pi * %d/%d * t)\n",n,A0,An,n,total,Bn,n,total);
    }
}
int main()
{   
    test();
    return 0;
}