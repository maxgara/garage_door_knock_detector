#include <stdio.h>
#include <math.h>

#define REAL 0
#define IMAG 1

const int period = 64; // must be power of 2
const int repeats_exp = 0; 
// const int total = period << repeats_exp; //total = period * (2 ^ repeats_exp)

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
        printf("%d %f\n",i,data[i]);
    }
}

void square_wave(double data[], double amp, int total)
{   
    for (int i = 0; i < total; i++)
    {
        if (i%(total/4) == 0)
        {
            amp = -amp;
        }
        data[i] = (double)amp;
    }
}
void sine_wave(double data[], double amp, int total){
    for (int i=0;i<total;i++){
        data[i] = amp * sin((((double)i)/total) * M_PI * 2);
    }
}
void cos_wave(double data[], double amp, int total){
    for (int i=0;i<total;i++){
        data[i] = amp * cos(((((double)i) + 0.1)/total) * M_PI * 2.01);
    }
}

void xsquared(double data[], double amp, int total)
{
    for (double i = 0; i < total; i++)
    {
        data[(int)i] = i*i;
    }
}

double invertFourierPoint(double eq[][3], int n, double t) {
    double sum=0;
    //loop thru functions
    for(int i=0;i<n;i++){
        double An = eq[i][0];
        double Bn = eq[i][1];
        double v = eq[i][2];
        // printf("An=%f B   n=%f, v=%f\n",An,Bn,v);
        double term = An*cos(2.0*M_PI*v*t) + Bn*sin(2.0*M_PI*v*t);
        // if (term*term <= 0.0001){
        //     continue;
        // }
        // printf("term %d of invert at %d:%f\n",i,(int)t,term);
        sum += term;
    }
    return sum;
}
//evaluate each of n equations eq at points 0 through count. store result in data.
void invertFourier(double eq[][3], int n, double data[], int count){
    //loop thru points
    for(int t=0; t<count;t++){
        data[t] = invertFourierPoint(eq, n, t);
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
    // int t = 3;
    double phase[2];
    // term(samples[t], 1, 100, 1, t, phase);
    // printf("term: %f + %f * i\n",phase[REAL], phase[IMAG]);
    // harm(samples, 1, 100, 1, phase);
    // printf("harm: %f + %f * i\n",phase[REAL], phase[IMAG]);

    double harmonicEqs[total][3];
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
        
        if (n==0){
            // printf("harmonic 0 (constant term): %f\n",An);
            continue;
        }
        harmonicEqs[n][0] = An;
        harmonicEqs[n][1] = Bn;
        harmonicEqs[n][2] = (double)n/total;
        if (An==0 && Bn==0){
            continue;
        }
        // printf("harmonic %d: %f*Cos(2pi * %d/%d * t) + %f*Sin(2pi * %d/%d * t)\n",n,An,n,total,Bn,n,total);
    }
    // printf("data:\n");
    print_data(samples,100,1);
    double invertdata[total];
    invertFourier(harmonicEqs,total,invertdata,total);
    for(int i=0;i<total;i++){
        // printf("harmonic EQ: %f %f %f\n",harmonicEqs[i][0],harmonicEqs[i][1],harmonicEqs[i][2]);
    }
    // printf("invert:\n");
    // print_data(invertdata,total,1);
    // double test = invertFourierPoint(harmonicEqs,3,37);
    // printf("test:%f\n",test);
}

int main()
{   
    test();
    return 0;
}
