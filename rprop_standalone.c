// MLP2 - TWO-LAYER MULTILAYER PERCEPTRON - SAMPLE IMPLEMENTATION (WITH THE XOR DATA)
// compile: gcc -Wall -std=gnu99 -O3 -ffast-math -funroll-loops -s -o MLP2 MLP2.c -lm
// Version 2.0 ----------------------------------------- Copyleft R.JAKSA 2009, GPLv3

#define Nin 2   // no. of inputs
#define Nh1 5   // no. of hidden units
#define Nou 1   // no. of outputs
#define Gamma 0.2 // learning rate
#define PositiveEta 1.2 // learning rate for positive rprop
#define NegativeEta 0.5 // learning rate for negative rprop
#define Dmax 5.0
#define Dmin 0.000001
#define Epochs 400 // no. of training epochs (cycles)

// ------------------------------------------------------------- END OF CONFIGURATION
#include <math.h> // fabs, exp
#include <stdlib.h> // rand, srand
#include <stdio.h>  // printf
#include <sys/timeb.h>  // ftime

#define Nx (1+Nin+Nh1+Nou) // no. of units
#define IN1 1   // 1st input
#define INn Nin   // last (n-th) input
#define H11 (Nin+1) // 1st hidden
#define H1n (Nin+Nh1) // last hidden
#define OU1 (H1n+1) // 1st output
#define OUn (H1n+Nou) // last output

typedef struct {  //
  double x[Nx];   // units inputs
  double y[Nx];   // units activations
  double delta[Nx]; // units delta signal
  double prevGrad[Nx][Nx]; // error value from previous time step
  double currGrad[Nx][Nx];
  double updateValue[Nx][Nx]; // update value for each weight
  double wDelta[Nx][Nx];
  double w[Nx][Nx]; // weights (single weights matrix)
  double dv[Nou]; // desired value on output
} ann_t;

#define w(i,j)  ann->w[i][j]
#define wDelta(i,j) ann->wDelta[i][j]
#define updateValue(i,j) ann->updateValue[i][j]
#define x(i)  ann->x[i]
#define y(i)  ann->y[i]
#define delta(i) ann->delta[i]
#define prevGrad(i,j) ann->prevGrad[i][j]
#define currGrad(i,j) ann->currGrad[i][j]
#define dv(i) ann->dv[i-OU1]

// block of units: for function definitions
#define blk_t(ann) ann_t *ann, int i1, int in, int j1, int jn
// block of units: for function calls
#define blk(BLKi,BLKj) BLKi##1,BLKi##n,BLKj##1,BLKj##n
// cycle through the layer: OU or H1 or IN
#define forlayer(BLK,i) for(int i=BLK##1; i<=BLK##n; i++)

// --------------------------------------- ACTIVATION FUNCTION AND ITS 1st DERIVATION
#define af(X) (1.0/(1.0+exp((-1.0)*X)))
#define df(X) (exp((-1.0)*X)/((1.0+exp((-1.0)*X))*(1.0+exp((-1.0)*X))))

// -------------------------------------------------------------------- AUX FUNCTIONS
#define min(a,b) (((a)<(b)) ? a : b)
#define max(a,b) (((a)>(b)) ? a : b)
//

int sign(double x) {
  if (x > 0) {
    return 1;
  } else if (x < 0) {
    return -1;
  } else {
    return 0;
  }
}


// ----------------------------------------------------------------------- INIT RPROP
void ann_initRprop(ann_t *ann) {
  // forlayer(OU, i) currGrad(i,j) = Dmin;
  // forlayer(H1, i) currGrad(i,j) = Dmin;
  // init every update value to 0.1
  for(int i=0; i<Nx; i++) {
    for(int j=0; j<Nx; j++) {
      currGrad(i,j) = 0.0;
      updateValue(i,j) = 0.1;
    }
  }
}
// --------------------------------------------------------------------- RESET DELTAS
void ann_resetDelta(ann_t *ann) {
  forlayer(OU, i) delta(i) = 0;
  forlayer(H1, i) delta(i) = 0;
}
// -------------------------------------------------------------- RANDOM WEIGHTS INIT
void ann_rndinit(ann_t *ann, double min, double max) {//
  y(0)=-1.0;            // the input for bias
  ann_initRprop(ann);
  ann_resetDelta(ann);
  for(int i=0; i<Nx; i++)
    for(int j=0; j<Nx; j++) //
      w(i,j) = rand() / (RAND_MAX/(max-min)) + min; 
} //

// ----------------------------------------------------------------- SINGLE LAYER RUN
static void layer_run(blk_t(ann)) {     // output/input block from-to
  for(int i=i1; i<=in; i++) {       // for every output
    x(i) = w(i,0) * y(0);       // add bias contribution
    for(int j=j1; j<=jn; j++) x(i) += w(i,j) * y(j);  // add main inputs contrib.
    y(i) = af(x(i)); }}         // apply activation function

// ---------------------------------------------------------------------- NETWORK RUN
void MLP2_run(ann_t *ann) {       //
  layer_run(ann,blk(H1,IN));        // in -> h1
  layer_run(ann,blk(OU,H1)); 
}        // h1 -> ou

// --------------------------------------------------- SINGLE LAYER BP WEIGHTS UPDATE
static void layer_update(blk_t(ann),double gamma) { //
  for(int i=i1; i<=in; i++) {       //
    w(i,0) += gamma * delta(i) * y(0);      // bias (weight) update
    for(int j=j1; j<=jn; j++)       //
      w(i,j) += gamma * delta(i) * y(j); }   // the weights update
  }   
// ---------------------------------------------------------------- COMPUTE GRADIENTS
  void calculate_gradients(blk_t(ann), int out) {
    for(int i=i1; i<=in; i++) {   
      currGrad(i,0) -= delta(i) * y(0);
      for(int j=j1; j<=jn; j++) {
        currGrad(i,j) -= delta(i) * y(j);
      }
    }
  }
// -------------------------------------------------------- SINGLE LAYER RPROP UPDATE
  void rprop_update(blk_t(ann)) {
  // printf("\n.:%f - %f:. ", prevDelta(OU1), delta(OU1));
    double errorChange;
    for(int i=i1; i<=in; i++) { 
      errorChange = prevGrad(i,0)*currGrad(i,0);
      if (errorChange > 0) {
        updateValue(i,0) = min(updateValue(i,0)*PositiveEta, Dmax);
        wDelta(i,0) = (-1.0) * sign(currGrad(i,0)) * updateValue(i,0);
        w(i,0) += wDelta(i,0);
        prevGrad(i,0) = currGrad(i,0);
        currGrad(i,0) = 0.0;
      } else if (errorChange < 0) {
        updateValue(i,0) = max(updateValue(i,0)*NegativeEta, Dmin);
        if (currGrad(i,0) > prevGrad(i,0)) {
          w(i,0) -= wDelta(i,0);
        }
        prevGrad(i,0) = 0.0;
        currGrad(i,0) = 0.0;
      } else {
        wDelta(i,0) = (-1.0) * sign(currGrad(i,0)) * updateValue(i,0);
        w(i,0) += wDelta(i,0);
        prevGrad(i,0) = currGrad(i,0);
        currGrad(i,0) = 0.0;
      }
      for(int j=j1; j<=jn; j++) {
        errorChange = prevGrad(i,j)*currGrad(i,j);
        if (errorChange > 0) {
          updateValue(i,j) = min(updateValue(i,j)*PositiveEta, Dmax);
          wDelta(i,j) = (-1.0) * sign(currGrad(i,j)) * updateValue(i,j);
          w(i,j) += wDelta(i,j);
        // printf("%f - %f", prevGrad(i,j), currGrad(i,j));
          prevGrad(i,j) = currGrad(i,j);
          currGrad(i,j) = 0.0;
        } else if (errorChange < 0) {
          updateValue(i,j) = max(updateValue(i,j)*NegativeEta, Dmin);
          if (currGrad(i,j) > prevGrad(i,j)) {
            w(i,j) -= wDelta(i,j);
          }
          prevGrad(i,j) = 0.0;
          currGrad(i,j) = 0.0;
        } else {
          wDelta(i,j) = (-1.0)*sign(currGrad(i,j)) * updateValue(i,j);
          w(i,j) += wDelta(i,j);
          prevGrad(i,j) = currGrad(i,j);
          currGrad(i,j) = 0.0;
        }
      }
    }
  }
// ------------------------------------------------- VANILLA BACKPROPAGATION LEARNING
void MLP2_vanilla_bp(ann_t *ann, double gamma) {  //
  MLP2_run(ann);          // 1st run the network
  forlayer(OU,i) {
    delta(i) = (dv(i)-y(i)) * df(x(i));  // delta on output layer
  }
  forlayer(H1,i) {          //
    double S=0.0; 
    forlayer(OU,h) S += delta(h) * w(h,i);
    delta(i) = S * df(x(i)); }        // delta on hidden layer
  layer_update(ann,blk(OU,H1),gamma);     // h1 -> ou
  layer_update(ann,blk(H1,IN),gamma); }     // in -> h1

// ------------------------------------------------------------ BASIC RPROP ALGORITHM
  void rprop_run(ann_t *ann, int num_of_patterns, int patternSet[4][3]) {
    int i;
    for (i = 0; i < num_of_patterns; i++) {
      y(IN1) = patternSet[i][0];
      y(IN1+1) = patternSet[i][1];
      dv(OU1) = patternSet[i][2];
      MLP2_run(ann);
      forlayer(OU,i) {
        delta(i) = (dv(i)-y(i)) * df(x(i));  // delta on output layer
      }
      forlayer(H1,i) {          //
        double S=0.0; 
        forlayer(OU,h) S += delta(h) * w(h,i);
        delta(i) = S * df(x(i)); 
      }
      calculate_gradients(ann, blk(OU,H1), 1);
      calculate_gradients(ann, blk(H1,IN), 0);
    }
    printf(":%f:", currGrad(OU1,0));
  // printf(" = %f", delta(OU1));
    rprop_update(ann,blk(OU,H1));
    rprop_update(ann,blk(H1,IN));
    ann_resetDelta(ann);
  }
// ----------------------------------------------------------------------------- MAIN
int XOR[4][3] = {{0,0,0,},{0,1,1,},{1,0,1,},{1,1,0,}};  // XOR data
double test_net(ann_t *ann) {
  double error = 0;
  y(IN1) = XOR[1][0];
  y(IN1+1) = XOR[1][1];
  dv(OU1) = XOR[1][2];
  MLP2_run(ann);
  error += 0.5 * (dv(OU1) - y(OU1)) * (dv(OU1) - y(OU1));
  y(IN1) = XOR[0][0];
  y(IN1+1) = XOR[0][1];
  dv(OU1) = XOR[0][2];
  MLP2_run(ann);
  error += 0.5 * (dv(OU1) - y(OU1)) * (dv(OU1) - y(OU1));
  y(IN1) = XOR[2][0];
  y(IN1+1) = XOR[2][1];
  dv(OU1) = XOR[2][2];
  MLP2_run(ann);
  error += 0.5 * (dv(OU1) - y(OU1)) * (dv(OU1) - y(OU1));
  y(IN1) = XOR[3][0];
  y(IN1+1) = XOR[3][1];
  dv(OU1) = XOR[3][2];
  MLP2_run(ann);
  error += 0.5 * (dv(OU1) - y(OU1)) * (dv(OU1) - y(OU1));
  return error;
}

int main(void) {          //
  FILE *f = fopen("rprop_stats.txt", "a");
  double error;
  ann_t ann[1];           //
  struct timeb t; ftime(&t); srand(t.time);   // time-seed random generator 
  ann_rndinit(ann,-0.1,0.1);        // initialize the network
  printf("\nEpoch:  Output  Des.out. (Error)\n"); //
  printf("--------------------------------\n");   //
  ftime(&t); long t1=t.time*1000+t.millitm;   // start time in milliseconds
  for(int epoch=0; epoch<=Epochs; epoch++) {    // for every epoch
    rprop_run(ann, 4, XOR);
    error = test_net(ann);
    fprintf(f, "%d %lf\n", epoch, error);
  }
  fclose(f);
  ftime(&t); long t2=t.time*1000+t.millitm;   // end time (in milliseconds)
  y(IN1) = XOR[1][0];
  y(IN1+1) = XOR[1][1];
  dv(OU1) = XOR[1][2];
  MLP2_run(ann);
  printf("\n | %f  %f |\n", dv(OU1), y(OU1));
  y(IN1) = XOR[0][0];
  y(IN1+1) = XOR[0][1];
  dv(OU1) = XOR[0][2];
  MLP2_run(ann);
  printf(" | %f  %f |\n", dv(OU1), y(OU1));
  y(IN1) = XOR[2][0];
  y(IN1+1) = XOR[2][1];
  dv(OU1) = XOR[2][2];
  MLP2_run(ann);
  printf(" | %f  %f |\n", dv(OU1), y(OU1));
  y(IN1) = XOR[3][0];
  y(IN1+1) = XOR[3][1];
  dv(OU1) = XOR[3][2];
  MLP2_run(ann);
  printf(" | %f  %f |\n", dv(OU1), y(OU1));
  printf("--------------------------------\n");   //
  long con=((Nin+1)*Nh1+(Nh1+1)*Nou)*4*Epochs;    // no. of connect. updated
  int msec=t2-t1; if(!msec) msec=1;     // time in milliseconds
  printf("%ld kCUPS in %.3f sec\n\n",con/msec,(double)msec/1000.0);
  for(int i = 0; i < Nx; i++) {
    for(int j = 0; j < Nx; j++) {
      printf("%f ", w(i,j));
    }
    printf("\n");
  }
  return(0); }            //

// ------------------------------------------------------------------------------ END
