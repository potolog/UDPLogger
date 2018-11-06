#include "datafile.h"
#include <cstdio>
#include <cstdlib>
#include <QDebug>

#include "signals.h"

DataFile::DataFile(QString path, QString version): m_path(path), m_version(version)
{
}

int DataFile::writeDataFile(QVector<QString>& names, QVector<QString>& units, QVector<QVector<double> > &data) const{

    if(names.length() != units.length()){
        return -2;
    }

    FILE * pFile;

    QByteArray ba = m_path.toLocal8Bit();
    char *path_c = ba.data();

    pFile = fopen (path_c, "wb");
    if(pFile==nullptr){
        return -1;
    }

    QByteArray header_ba = createHeader(names,units).toUtf8();
    fwrite(header_ba.data(),sizeof(char), header_ba.length(),pFile ); // possible to read data in another thread

    char seperator = ',';
    char new_line = '\n';

    for(int row = 0; row < data.length(); row++){
        for(int column = 0; column < data[0].length(); column++){
            double value = data[row][column];
            fwrite(&value,sizeof(double),1,pFile);
            //fwrite(&seperator, sizeof(char),1,pFile);
        }
        //fwrite(&new_line,sizeof (char),1,pFile);
    }
    fclose (pFile);
    return 0;

}

// signal names and units are comma seperated
int DataFile::readDataFile(){
    FILE* pFile;

    QByteArray ba = m_path.toLocal8Bit();
    char *path_c = ba.data();

    pFile = fopen(path_c, "rb");
    if(pFile== nullptr){
        return -1;
    }

    // get line can be used, because in the header there are only ascii characters
    size_t size=0;
    char* lineptr = nullptr;
    if(getline(&lineptr, &size,pFile) < 0){ // header
        // problem
        return -2;
    }
    if(parseHeader(lineptr)== -1){
        return -3;
    }

    if(getline(&lineptr, &size, pFile) < 0){// signal names and units
        return -4;
    }
    QString names_units(lineptr);
    names_units.replace("\n","");
    names_units.replace(" ","");
    QStringList name_units_list = names_units.split(",");
    if(name_units_list.length()%2 != 0){ // problem, when there are not the same number of names and units
        return -5;
    }
    m_signals.resize(name_units_list.length()/2);
    for(int i=0; i<name_units_list.length(); i+=2){
        struct dataFileSignal signal;
        signal.name = name_units_list[i];
        signal.unit = name_units_list[i+1];

        m_signals[i/2] = signal;
    }
    free(lineptr); // because getline allocate memory

    int result = -1;


    int element_size = sizeof(double); //+1;
    int buffer_size = datafile::read_buffer_size_factor*m_signals.length()*element_size;
    char* buffer = (char*) std::malloc(buffer_size);
    if(buffer == nullptr){
        qDebug() << "Not able to reserve " << buffer_size << "Bytes.";
        return -4;
    }
    while (result = fread(buffer,sizeof(char),buffer_size,pFile)) {
        //memcpy(&temp[temp_counter],buffer,datafile::read_buffer_size/2);
        //temp_counter+=datafile::read_buffer_size*m_signals.lengt()*(sizeof(double)+1);
        int block_size = element_size*m_signals.length();
        int block_count = result/block_size;
        for(int i=0; i<block_count; i+=block_size){
            int signal = 0;
            for(int j=0; j<block_size; j+= element_size){
                double value = *((double*)(&buffer[i+j]));
                m_signals[signal].data.append(value);
                signal++;
            }
        }
    }
    free(buffer);
    fclose(pFile);

    return 0;
}
// Example Header: "DataFile(.df),Version1.0"
// assumption: if after the version number some other text comes,
// this text is comma seperated from the version number!
int DataFile::parseHeader(char* lineptr){
    QString header(lineptr);

    QString part = "";
    for(int i=0; i< datafile::filetype.length(); i++){
        part += header[i];
    }
    if(part.compare(datafile::filetype)!=0){
        return -1;
    }

    part = "";
    for(int i=datafile::filetype.length(); i<11; i++){
        part += header[i];
    }
    if(part.contains("Version")){
        m_version = part.split("Version")[1].split(",")[0];
    }
    return 0;
}

QString DataFile::createHeader(const QVector<QString> &names, const QVector<QString> &units)const{
    QString header = datafile::filetype+",Version"+m_version + "\n";

    QString names_units = "";
    for(int i=0; i< names.length(); i++){
        names_units += names[i]+","+units[i];
        if(i<names.length()-1){
            names_units += ",";
        }
    }
    names_units += "\n";
    header+= names_units;

    return header;
}
