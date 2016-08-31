/**
 @author Nicholas Gillian <nick@nickgillian.com>
 @brief This file implements a basic tool for training a linear regression model. The dataset used to train the model can be in two formats, (1) a GRT RegressionData formatted file or (2)
 a CSV formatted file.  If the data is formatted as a CSV file then it should be formatted as follows:
 - each row should contain a sample
 - the first N columns should contain the input attributes (a.k.a. features)
 - the last T columns should contain the target attributes
 - columns should be seperated by a comma delimiter ','
 - rows should be ended with a new line operator '\n'
 Note, if the CSV option is used, then the user must also specifiy the number of input dimensions and number of target dimensions via the command line options (-n and -t respectively). These
 additional arguments are not required if the GRT RegressionData file format is used (as this information is contained in the meta data section of the file).
*/

//You might need to set the specific path of the GRT header relative to your project
#include <GRT/GRT.h>
using namespace GRT;
using namespace std;

InfoLog infoLog("[grt-lin-reg-tool]");
WarningLog warningLog("[WARNING grt-lin-reg-tool]");
ErrorLog errorLog("[ERROR grt-lin-reg-tool]");

bool printUsage(){
    infoLog << "grt-lin-reg-tool [options]\n";
    infoLog << "\t-f: sets the filename the training data will be loaded from. The training data can either be a GRT RegressionData file or a CSV file. \n";
    infoLog << "\t-n: sets the number of input dimensions in the dataset, only required if the input data format is a CSV file. \n";
    infoLog << "\t-t: sets the number of target dimensions in the dataset, only required if the input data format is a CSV file. \n";
    infoLog << "\t--model: sets the filename the regression model will be saved to\n";
    infoLog << endl;
    return true;
}

bool train( CommandLineParser &parser );

int main(int argc, char * argv[])
{

    if( argc < 2 ){
        errorLog << "Not enough input arguments!" << endl;
        printUsage();
        return EXIT_FAILURE;
    }

    //Create an instance of the parser
    CommandLineParser parser;

    //Disable warning messages
    parser.setWarningLoggingEnabled( false );

    //Add some options and identifiers that can be used to get the results
    parser.addOption( "-f", "filename" );
    parser.addOption( "-n", "num-input-dimensions" );
    parser.addOption( "-t", "num-target-dimensions" );
    parser.addOption( "--model", "model-filename" );

    //Parse the command line
    parser.parse( argc, argv );

    //Train the model model
    if( train( parser ) ){
      infoLog << "Model Trained!" << endl;
      return EXIT_SUCCESS;
    }

    errorLog << "Failed to train model!" << endl;
    printUsage();

    return EXIT_FAILURE;
}

bool train( CommandLineParser &parser ){

    infoLog << "Training regression model..." << endl;

    string trainDatasetFilename = "";
    string modelFilename = "";
    string defaultFilename = "linear-regression-model.grt";

    //Get the filename
    if( !parser.get("filename",trainDatasetFilename) ){
        errorLog << "Failed to parse filename from command line! You can set the filename using the -f." << endl;
        printUsage();
        return false;
    }

    //Get the model filename
    parser.get("model-filename",modelFilename,defaultFilename);

    //Load the training data to train the model
    RegressionData trainingData;

    //Try and parse the input and target dimensions
    unsigned int numInputDimensions = 0;
    unsigned int numTargetDimensions = 0;
    if( parser.get("num-input-dimensions",numInputDimensions) && parser.get("num-target-dimensions",numTargetDimensions) ){
      infoLog << "num input dimensions: " << numInputDimensions << " num target dimensions: " << numTargetDimensions << endl;
      trainingData.setInputAndTargetDimensions( numInputDimensions, numTargetDimensions );
    }

    infoLog << "- Loading Training Data..." << endl;
    if( !trainingData.load( trainDatasetFilename ) ){
        errorLog << "Failed to load training data!\n";
        return false;
    }

    const unsigned int N = trainingData.getNumInputDimensions();
    const unsigned int T = trainingData.getNumTargetDimensions();
    infoLog << "- Num training samples: " << trainingData.getNumSamples() << endl;
    infoLog << "- Num input dimensions: " << N << endl;
    infoLog << "- Num target dimensions: " << T << endl;

    //Create a new regression instance
    LinearRegression regression;

    regression.setMaxNumEpochs( 500 );
    regression.setMinChange( 1.0e-5 );
    regression.setUseValidationSet( true );
    regression.setValidationSetSize( 20 );
    regression.setRandomiseTrainingOrder( true );
    regression.enableScaling( true );

    //Create a new pipeline that will hold the regression algorithm
    GestureRecognitionPipeline pipeline;

    //Add a multidimensional regression instance and set the regression algorithm to Linear Regression
    pipeline.setRegressifier( MultidimensionalRegression( regression, true ) );

    infoLog << "- Training model...\n";

    //Train the classifier
    if( !pipeline.train( trainingData ) ){
        errorLog << "Failed to train model!" << endl;
        return false;
    }

    infoLog << "- Model trained!" << endl;

    infoLog << "- Saving model to: " << modelFilename << endl;

    //Save the pipeline
    if( pipeline.save( modelFilename ) ){
        infoLog << "- Model saved." << endl;
    }else warningLog << "Failed to save model to file: " << modelFilename << endl;

    infoLog << "- TrainingTime: " << pipeline.getTrainingTime() << endl;

    return true;
}

