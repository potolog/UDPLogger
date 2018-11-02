#ifndef DATAFILE_H
#define DATAFILE_H

#include <QString>
#include <QVector>

class Signals;

struct dataFileSignal{
    QString name;
    QString unit;
    QVector<double> data;
};

namespace datafile {
    const QString filetype = "DataFile(.df)";
    const int read_buffer_size_factor = 100;
}

class DataFile
{
public:
    DataFile(QString path, QString version = "1.0");
    QString getVersion(){return m_version;}
    int writeDataFile(QVector<QString>& names, QVector<QString>& units, QVector<QVector<double>> &data) const;
    int readDataFile();
    QString getSignalName(int index){return m_signals.at(index).name;}
    QString getUnit(int index){return m_signals.at(index).unit;}
    struct dataFileSignal* getSignal(int index){return &m_signals[index];}
    QString createHeader(const QVector<QString>& names, const QVector<QString> &units) const;
    int parseHeader(char *lineptr);

private:
    QString m_version;
    QString m_path;
    QVector<struct dataFileSignal> m_signals;
};

/* Test:
DataFile datafile("/home/martin/test.df");

const int number_signals = 10;
const int number_elements = 1;
QVector<QString> names;
names.resize(number_signals);
QVector<QString> units;
units.resize(number_signals);
QVector<QVector<double>> data;
data.resize(number_elements);

for(int i=0; i< number_signals; i++){
    names[i] = "Signal"+QString::number(i);
    units[i] = "Unit"+QString::number(i);
}

QVector<double> vec;
vec.resize(number_signals);
int k=24;
for(int i=0; i<number_elements; i++){
    if(k>24){
        k=24;
    }
    for(int j=0; j<number_signals; j++){

        if(j!=0){
        vec[j] = (static_cast<double>(k)*number_elements*1000*5*2+j)/(j);
        }else{
            vec[j] = (static_cast<double>(k)*number_elements*1000*5*2+j);
        }
    }
    k++;
    data[i] = vec;
}
datafile.writeDataFile(names,units,data);

DataFile datafile2("/home/martin/test.df");
int errorcode = datafile2.readDataFile();
if(errorcode < 0){
    qDebug() << "Fehlercode: " << errorcode;
}
bool end = false;
for(int row =0; row < number_elements; row++){
    for(int column = 0; column < number_signals; column++){
        if(data[row][column] != datafile2.getSignal(column)->data[row]){
            qDebug() << "Row: " << row << ", Column: " << column;
            end = true;
            break;
        }
    }
    if(end)
        break;
}
qDebug() << "Finish";*/

#endif // DATAFILE_H
