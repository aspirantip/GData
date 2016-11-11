#ifndef NNMODEL_H
#define NNMODEL_H


#include <QObject>
#include <QDebug>

#include <sys/stat.h>
#include "Include/Eval.h"

#include <iostream>


using namespace Microsoft::MSR::CNTK;

// Used for retrieving the model appropriate for the element type (float / double)
template<typename ElemType>
using GetEvalProc = void(*)(IEvaluateModel<ElemType>**);

typedef std::pair<std::wstring, std::vector<float>*> MapEntry;
typedef std::map<std::wstring, std::vector<float>*>  Layer;


class NNModel : public QObject
{
    Q_OBJECT

private:
    IEvaluateModel<float> *model;
    std::string modelFilePath;

    std::map<std::wstring, size_t> inDims;
    std::map<std::wstring, size_t> outDims;

    size_t sizeInputVector;
    size_t sizeOutputVector;

    float_t probability;

    void createNetwork (std::string networkConfiguration);

    void setSizeInputVector( size_t size_vector );
    void setSizeOutputVector( size_t size_vector );

public:
    explicit NNModel(QObject *parent = 0);
    ~NNModel();

    bool loadModel (std::string pathModel);
    bool ClassificationImage(std::vector<float> image);
    size_t getSizeInputVector ();
    size_t getSizeOutputVector ();


signals:

public slots:
    void setThreshold(double probImage);
};

#endif // NNMODEL_H
