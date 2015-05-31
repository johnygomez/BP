/** @file
 * @author Jan Gamec
 * @date 24 May 2015
 * @brief File containing implementation of MLP with RPROP learning
 *
 * This module contain functions for initialization, running and training Multilayer Perceptron
 * with RPROP learning algorithm. This file cannot be run independently, but
 * is used as a library for python wrapper. File can be combiled:
 * gcc -Wall -std=gnu99 -O3 -ffast-math -funroll-loops -s -o rprop_standalone rprop.c -lm
 */
/**Defines number of input neurons. This needs to be changed according to task.*/ 
#define Nin 5
/**Defines number of neurons in first hidden layer. This needs to be changed according to task.*/
#define Nh1 10
/**Defines number of neurons in second hidden layer. This needs to be changed according to task.*/
#define Nh2 10
/**Defines number of output neurons. This needs to be changed according to task.*/
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

/**
 * @brief Struct representing a neural network.
 *
 * Container for a complex MLP structure.
 */
typedef struct {  //
  double x[Nx];   /**< Input of neurons*/
  double y[Nx];   /**< Output of neurons*/
  double delta[Nx]; /**< Delta value on neurons*/
  double prevGrad[Nx][Nx]; /**< Weight error gradient from last step*/
  double currGrad[Nx][Nx]; /**< Weight current error gradient */
  double updateValue[Nx][Nx]; /**< Update value according to RPROP algorithm for each weight*/
  double wDelta[Nx][Nx]; /**< Delta of weights change*/
  double w[Nx][Nx]; /**< Weights matrix*/
  double dv[Nou]; /**< Target value on output neurons*/
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

/** 
 @brief Returns the sign of given double
 @param x Double precission number
 */
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
/** @brief Shuffles the given 2D array
 *  @param[in,out] array Array to be shuffled
 *  @param n length of an array
 */
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
/** @brief Rprop variables initialization.
 *  Current weights gradient is set to 0 and udpate value to 0.1
 *  @param[in,out] ann Neural network structure
 */
void ann_initRprop(ann_t *ann) {
  // init every update value to 0.1
  for(int i=0; i<Nx; i++) {
    for(int j=0; j<Nx; j++) {
      currGrad(i,j) = 0.0;
      updateValue(i,j) = 0.1;
    }
  }
}
// --------------------------------------------------------------------- RESET DELTAS
/** @brief Resets all delta values on neurons
 *  @param[in,out] ann Neural network structure
 */
void ann_resetDelta(ann_t *ann) {
  forlayer(OU, i) delta(i) = 0;
  forlayer(H2, i) delta(i) = 0;
  forlayer(H1, i) delta(i) = 0;
}
// -------------------------------------------------------------- RANDOM WEIGHTS INIT
/** @brief Randomly initializes weights matrix withit given interval
 *  @param[in,out] ann Neural network structure
 *  @param min Bottom weight bound
 *  @param max Upper weight bound
 */
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
/** @brief Initializes a network from a give weights matrix
 *  @param[in,out] ann Neural network structure
 *  @param weights Initializatioon weights matrix
 */
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
/** @brief Calculates an output on one layer using output from previous
 *  @param[in,out] blk_t(ann) Macro separating 2 layers from ann structure
 */
static void layer_run(blk_t(ann)) {     // output/input block from-to
  for(int i=i1; i<=in; i++) {       // for every output
    x(i) = w(i,0) * y(0);       // add bias contribution
    for(int j=j1; j<=jn; j++) x(i) += w(i,j) * y(j);  // add main inputs contrib.
      y(i) = af(x(i)); 
  }
}
// ---------------------------------------------------------------------- NETWORK RUN
/** @brief Simple runs of network in a forward direction
 *  Calculate output running whole network forward
 *  @param[in,out] ann Neural network structure
 */
void MLP2_run(ann_t *ann) {       
  layer_run(ann, blk(H1,IN));      // in -> h1
  layer_run(ann, blk(H2,H1));      // h1 -> h2
  layer_run(ann, blk(OU,H2));      // h2 -> ou
}         
// ---------------------------------------------------------------- COMPUTE GRADIENTS
/** @brief Calculate weights gradients between 2 layers
 *  
 *  @param blk_t(ann) Macro representing separated 2 layers
 *  @param out Signalizes whether the layer is hidden or output/input
 */
void calculate_gradients(blk_t(ann), int out) {
  for(int i=i1; i<=in; i++) {   
    currGrad(i,0) -= delta(i) * y(0);
    for(int j=j1; j<=jn; j++) {
      currGrad(i,j) -= delta(i) * y(j);
    }
  }
}
// --------------------------------------------------------------- SINGLE NETWORK RUN
/** @brief Runs a network in a forward direction with a given input pattern
 *  Calculate output running whole network forward
 *  @param[in,out] ann Neural network structure
 *  @param pattern Training pattern
 *  @return Vector of values on the output neurons
 */
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
/** @brief Implementation of RPROP learning algorithm according to paper
 *  Update rules and equations are described in work. This function updates weights
 *  between 2 layers.
 *  @param blk_t(ann) Macro separating 2 following layers
 */
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
/** @brief RPROP learning step.
 *  Implementation of RPROP algorithm according to paper. This method makes one 
 *  forward run throught network calculating learning variables and updating weights
 *  after then.
 *  @param[in,out] ann Neural network structure
 */
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
/** @brief Tests a network for an error against training set
 * 
 *  @param[in,out] ann Neural network structure
 *  @param num_of_pattern Number of pattern in training set
 *  @param patternSet Training set represented by 2D array
 */
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
  printf("Error: %f\n", error);
}

/** @brief Manages a learning process
 *  Repeats learning procedure for the given number of epochs and shuffles the
 *  training set. It tests the network after then.
 *  @param[in,out] ann Neural network structure
 *  @param num_of_epochs Number of training epochs
 *  @param num_of_patterns Number of training patterns
 *  @param patternSet Training set represented by a 2D array
 */
void rprop_learn(ann_t *ann, 
  int num_of_epochs, 
  int num_of_patterns, 
  double** patternSet) {
  for (int epoch = 0; epoch <= num_of_epochs; epoch++) {
    rprop_learning_step(ann, num_of_patterns, patternSet);
    shuffle(patternSet, num_of_patterns);
  }
  test_net(ann, num_of_patterns, patternSet);
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
