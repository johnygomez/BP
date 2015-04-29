// MLP2 - TWO-LAYER MULTILAYER PERCEPTRON - SAMPLE IMPLEMENTATION (WITH THE XOR DATA)
// compile: gcc -Wall -std=gnu99 -O3 -ffast-math -funroll-loops -s -o MLP2 MLP2.c -lm
// Version 2.0 ----------------------------------------- Copyleft R.JAKSA 2009, GPLv3

#define Nin 6  // no. of inputs
#define Nh1 20   // no. of hidden units in layer 1
#define Nh2 20   // no. of hidden units in layer 2
#define Nou 1   // no. of outputs
#define Gamma 0.2 // learning rate
#define PositiveEta 1.2 // learning rate for positive rprop
#define NegativeEta 0.5 // learning rate for negative rprop
#define Dmax 5.0
#define Dmin 0.000001
#define Epochs 300 // default no. of training epochs (cycles)

// ------------------------------------------------------------- END OF CONFIGURATION
#include <math.h> // fabs, exp
#include <stdlib.h> // rand, srand
#include <stdio.h>  // printf
#include <sys/timeb.h>  // ftime -> not needed

#define Nx (1+Nin+Nh1+Nh2+Nou) // no. of units
#define IN1 1   // 1st input
#define INn Nin   // last (n-th) input
#define H11 (Nin+1) // 1st hidden 1
#define H1n (Nin+Nh1) // last hidden 1
#define H21 (H1n+1) // 1st hidden 2
#define H2n (H1n+Nh2) // last hidden 2
#define OU1 (H2n+1) // 1st output
#define OUn (H2n+Nou) // last output

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

// --------------------------------------------------------------- SHUFFLE PATTERNSET
void shuffle(double **array, int n)
{
  if (n > 1) 
  {
    int i;
    for (i = 0; i < n - 1; i++) 
    {
      int j = i + rand() / (RAND_MAX / (n - i) + 1);
      double* t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
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
  forlayer(H2, i) delta(i) = 0;
  forlayer(H1, i) delta(i) = 0;
}
// -------------------------------------------------------------- RANDOM WEIGHTS INIT
void ann_rndinit(ann_t *ann, double min, double max) {//
  y(0)=-1.0;            // the input for bias
  ann_initRprop(ann);
  ann_resetDelta(ann);
  for(int i=0; i<Nx; i++)
    for(int j=0; j<Nx; j++) //
      // replace rand with srand(time(NULL))
      w(i,j) = rand() / (RAND_MAX/(max-min)) + min; 
} 
// ------------------------------------------------------------ INIT NET WITH WEIGHTS
void ann_init(ann_t* ann, double** weights) {
  y(0)=-1.0;            // the input for bias
  ann_initRprop(ann);
  ann_resetDelta(ann);
  for(int i=0; i<Nx; i++) {
    for(int j=0; j<Nx; j++) {
      w(i,j) = weights[i][j];
    }
  }
}
// ----------------------------------------------------------------- SINGLE LAYER RUN
static void layer_run(blk_t(ann)) {     // output/input block from-to
  for(int i=i1; i<=in; i++) {       // for every output
    x(i) = w(i,0) * y(0);       // add bias contribution
    for(int j=j1; j<=jn; j++) x(i) += w(i,j) * y(j);  // add main inputs contrib.
      y(i) = af(x(i)); 
  }
}
// ---------------------------------------------------------------------- NETWORK RUN
void MLP2_run(ann_t *ann) {       
  layer_run(ann, blk(H1,IN));      // in -> h1
  layer_run(ann, blk(H2,H1));      // h1 -> h2
  layer_run(ann, blk(OU,H2));      // h2 -> ou
}         
// ---------------------------------------------------------------- COMPUTE GRADIENTS
// TODO remove out param??
void calculate_gradients(blk_t(ann), int out) {
  for(int i=i1; i<=in; i++) {   
    currGrad(i,0) -= delta(i) * y(0);
    for(int j=j1; j<=jn; j++) {
      currGrad(i,j) -= delta(i) * y(j);
    }
  }
}
// --------------------------------------------------------------- SINGLE NETWORK RUN
double *rprop_run(ann_t *ann, double* pattern) {
  int i,j;
  double *res = (double*)malloc(sizeof(double) * Nou);
  for(i = 0; i < Nin; i++) {
    y(IN1+i) = pattern[i];  
  }
  for(j = 0; j < Nou; j++) {
    dv(OU1 + j) = pattern[i++];
  }
  layer_run(ann, blk(H1,IN));      // in -> h1
  layer_run(ann, blk(H2,H1));      // h1 -> h2
  layer_run(ann, blk(OU,H2));      // h2 -> ou
  for (int i = 0; i < Nou; i++) {
    res[i] = y(i + OU1);
  }
  return res;
}
// -------------------------------------------------------- SINGLE LAYER RPROP UPDATE
void rprop_update(blk_t(ann)) {
  double errorChange;
  for(int i=i1; i<=in; i++) { 
    errorChange = prevGrad(i,0)*currGrad(i,0);
    // update neurons bias weights
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
    // update weights between neurons
    for(int j=j1; j<=jn; j++) {
      errorChange = prevGrad(i,j)*currGrad(i,j);
      if (errorChange > 0) {
        updateValue(i,j) = min(updateValue(i,j)*PositiveEta, Dmax);
        wDelta(i,j) = (-1.0) * sign(currGrad(i,j)) * updateValue(i,j);
        w(i,j) += wDelta(i,j);
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

// ------------------------------------------------------------ BASIC RPROP ALGORITHM
void rprop_learning_step(ann_t *ann, int num_of_patterns, double** patternSet) {
  int i;
  for (i = 0; i < num_of_patterns; i++) {
    rprop_run(ann, patternSet[i]);
    forlayer(OU,i) {
      delta(i) = (dv(i)-y(i)) * df(x(i));  // delta on output layer
    }
    forlayer(H2,i) {          //
      double S=0.0; 
      forlayer(OU,h) S += delta(h) * w(h,i);
      delta(i) = S * df(x(i)); 
    }
    forlayer(H1,i) {          //
      double S=0.0; 
      forlayer(H2,h) S += delta(h) * w(h,i);
      delta(i) = S * df(x(i)); 
    }
    calculate_gradients(ann, blk(OU,H2), 1);
    calculate_gradients(ann, blk(H2,H1), 1);
    calculate_gradients(ann, blk(H1,IN), 0);
  }
    rprop_update(ann,blk(OU,H2));
    rprop_update(ann,blk(H2,H1));
    rprop_update(ann,blk(H1,IN));
    ann_resetDelta(ann);
  }
// ----------------------------------------------------------- RPROP LEARNING MANAGER
void test_net(ann_t *ann, int num_of_patterns, double** patternSet) {
  double error = 0;
  int j;
  for (int i = 0; i < num_of_patterns-1; i++) {
    for(j = 0; j < Nin; j++) {
      y(IN1+j) = patternSet[i][j];  
    }
    dv(OU1) = patternSet[i][j];
    MLP2_run(ann);
    error += 0.5 * (dv(OU1) - y(OU1)) * (dv(OU1) - y(OU1));
  }
  printf("error: %f\n", error);
}

void rprop_learn(ann_t *ann, 
  int num_of_epochs, 
  int num_of_patterns, 
  double** patternSet) {
  for (int epoch = 0; epoch <= num_of_epochs; epoch++) {
    rprop_learning_step(ann, num_of_patterns, patternSet);
    shuffle(patternSet, num_of_patterns);
    test_net(ann, num_of_patterns, patternSet);
  }
}
// ----------------------------------------------------------------------------- MAIN
// int main(void) {
//   ann_t ann[1];
//   ann_rndinit(ann,-0.1,0.1);        // initialize the network
//   for(int epoch=0; epoch<=Epochs; epoch++) {    // for every epoch
//     rprop_run(ann, XOR[1]);
//   }
//   return(0); 
// }
// ------------------------------------------------------------------------------ END
