#include "nnmodel.h"


NNModel::NNModel(QObject *parent) : QObject(parent)
{
    probability = 0.0;

}

NNModel::~NNModel()
{

}

bool NNModel::loadModel( std::string pathModel)
{
    struct stat statBuf;
    if (stat(pathModel.c_str(), &statBuf) != 0)
    {
        fprintf (stderr, "Error: The model %s does not exist.\n", pathModel.c_str());
        return (false);
    }
    fprintf (stdout, "Loaded model: %s.\n", pathModel.c_str());


    GetEvalF( &model );

    // Load model with desired outputs
    std::string networkConfiguration;

    // Uncomment the following line to re-define the outputs (include h1.z AND the output ol.z)
    // When specifying outputNodeNames in the configuration, it will REPLACE the list of output nodes
    // with the ones specified.
    //networkConfiguration += "outputNodeNames=\"h1.z:ol.z\"\n";
    networkConfiguration += "modelPath=\"" + pathModel + "\"";
    std::cout << "Network configuration: " << networkConfiguration << std::endl;

    createNetwork( networkConfiguration );

    return true;
}

void NNModel::createNetwork(std::string networkConfiguration)
{
    model->CreateNetwork( networkConfiguration );

    // get the model's layers dimensions
    model->GetNodeDimensions(inDims, NodeGroup::nodeInput);
    model->GetNodeDimensions(outDims, NodeGroup::nodeOutput);

    setSizeInputVector( inDims[inDims.begin()->first] );
}

void NNModel::setSizeInputVector(size_t size_vector)
{
    sizeInputVector = size_vector;
}

bool NNModel::ClassificationImage(std::vector<float> image)
{
    // Allocate the output values layer
    std::vector<float> outputs;

    // Setup the maps for inputs and output
    Layer inputLayer;
    auto inputLayerName = inDims.begin()->first;
    inputLayer.insert( MapEntry(inputLayerName, &image) );

    Layer outputLayer;
    auto outputLayerName = outDims.begin()->first;
    outputLayer.insert( MapEntry(outputLayerName, &outputs) );

    // We can call the evaluate method and get back the results (single layer)...
    model->Evaluate(inputLayer, outputLayer);

    // Output the results
    fprintf(stderr, "Layer '%ls' output:\n", outputLayerName.c_str());
    fprintf(stderr, "%f\n", outputs[0]);



    return outputs[0] >= probability ? true : false;
}

size_t NNModel::getSizeInputVector()
{
    return sizeInputVector;
}

void NNModel::setThreshold(double probImage)
{
    qDebug() << "NNModel: Threshold =" << probImage;

    probability = probImage;
}
