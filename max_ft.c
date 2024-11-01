#include <stdio.h>
#include <math.h>

#define REAL 0
#define IMAG 1

const int period = 64; // must be power of 2
const int repeats_exp = 0; 
// const int total = period << repeats_exp; //total = period * (2 ^ repeats_exp)

void square_wave(double data[]);
// void sine_wave(double data[], int total);
void print_data(double data[], int include_zeros);
void term(double sk, double n, double P, double T, int t, double out[]);



//return magnitude of term t in sum for harmonic n
// sk is function sample, P is total sample length, n is harmonic number, T is delta time for sampling, t is time
void term(double sk, double n, double P, double T, int t, double out[]){
    double c1 = T * sk;
    double inner = -2 * M_PI * n/P;
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
    }
    out[0] = (T/P) * sum[0];
    out[1] = (T/P) * sum[1];
    return sqrt(out[REAL] * out[REAL] + out[IMAG] * out[IMAG]);
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
    int t = 3;
    double phase[2];
    term(samples[t], 1, 100, 1, t, phase);
    printf("term: %f + %f * i\n",phase[REAL], phase[IMAG]);
    for(int n=0;n<total;n++){
    

    harm(samples,n,total,1,phase);
    printf("harm %d: %f + %f * i\n",n, phase[REAL], phase[IMAG]);
}
    
}
int main()
{   
    test();
    return 0;
}