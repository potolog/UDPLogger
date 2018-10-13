/***
 *  This file is part of UDPLogger
 *
 *  Copyright (C) 2018 Martin Marmsoler, martin.marmsoler at gmail.com
 *
 *  UDPLogger is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with UDPLogger.  If not, see <http://www.gnu.org/licenses/>.
 ***/

#include "signals.h"
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <xlnt/xlnt.hpp>
#include <iostream>




Signals::Signals(){
    m_c_path = "";
    m_header_path = "";
}

void Signals::importJSonFile(QString filename){

    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject object = d.object();

    if(object.contains("Signals")){
        QJsonArray imported_signals = object["Signals"].toArray();
        QVector<struct Signal> signal_vector;
        struct Signal signal_element;

        for(int i=0; i<imported_signals.count(); i++){
            QJsonObject signal = imported_signals[i].toObject();
            signal_element.datatype = signal["Datatype"].toString();
            signal_element.index = signal["Index"].toInt();
            signal_element.offset = signal["Offset"].toInt();
            signal_element.name = signal["Signalname"].toString();
            signal_vector.append(signal_element);
        }
        setSignals(&signal_vector);

        emit signalsChanged();
    }


}

int Signals::importXLSX(QString filename){

    QVector<struct Signal> new_signals;

    xlnt::workbook wb;
    wb.load(filename.toStdString());
    auto ws = wb.active_sheet();

    std::vector< std::vector<std::string> > theWholeSpreadSheet;
     for (auto row : ws.rows(false)){
        std::vector<std::string> aSingleRow;
         for (auto cell : row){
             aSingleRow.push_back(cell.to_string());
         }
         theWholeSpreadSheet.push_back(aSingleRow);
     }

     unsigned long description_row = -1;
     for (unsigned long i=0; i < theWholeSpreadSheet.size(); i++){
         auto row = theWholeSpreadSheet.at(i);
         if(row[0].compare("Index")==0){
            description_row = i;
         }
     }

     if(description_row <0){
        return -1;
     }

     // Find columns where
     long datatype_column = -1;
     long index_column = 0;
     long name_column = -1;
     long size_column = -1;
     for(unsigned long column=0; column < theWholeSpreadSheet.at(description_row).size(); column++){
         std::string column_value = theWholeSpreadSheet.at(description_row).at(column);
         if(column_value.compare("Datatype")== 0){
             datatype_column = column;
             continue;
         }

         if(column_value.compare("Signalname")==0){
             name_column = column;
             continue;
         }

         if(column_value.compare("Size [Bytes]")==0){
             size_column = column;
             continue;
         }
     }

     if(datatype_column <0 || index_column < 0 || name_column < 0|| size_column<0){
         return -2;
     }

     int offset = 0;

     for(unsigned long i=description_row+1; i< theWholeSpreadSheet.size(); i++){
         struct Signal signal;
         signal.datatype = QString::fromStdString(theWholeSpreadSheet.at(i).at(datatype_column));
         signal.name = QString::fromStdString(theWholeSpreadSheet.at(i).at(name_column));
         signal.index = std::stoi(theWholeSpreadSheet.at(i).at(index_column));
         signal.offset = offset;
         offset+= std::stoi(theWholeSpreadSheet.at(i).at(size_column));
         new_signals.append(signal);
     }

    m_signals.clear();
    m_signals = new_signals;
    emit signalsChanged();
    return 0;
}

void Signals::importSignals(){
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open Signals file"), "/home", tr("SignalFiles(*.udpLoggerSignals, *.xlsx)"));
    if(fileName.split(".").last() == "xlsx"){
        importXLSX(fileName);
        return;
    }
    importJSonFile(fileName);
}

void Signals::exportUDPFunction(){

    QString pathWithFileName = QFileDialog::getSaveFileName(nullptr,
            tr("Export C Funktion"), m_c_path,
            tr("C/C++ File (*.c, *.cpp)"));

    QStringList tempListPath = pathWithFileName.split("/");
    QString file_name = tempListPath.last();
    tempListPath.removeLast();
    QString path = tempListPath.join("/");
    m_c_path = path;

    QStringList tempList = file_name.split(".");
    QString fileNameHeader = "";
    QString fileNameDeclaration = "";

    if(tempList.length() == 1){
        fileNameHeader = file_name + ".h";
        fileNameDeclaration = file_name + ".cpp";
    }else{
       fileNameDeclaration = file_name;
       for (int i=0; i< tempList.length()-1; i++){ // last element of tempList is extension
            fileNameHeader+= tempList[i];
       }
       fileNameHeader+= ".h";
    }

    QFile saveFileDefinition(m_c_path+"/"+m_header_path+"/"+fileNameHeader);
    if (!saveFileDefinition.open(QIODevice::WriteOnly)) {
       qWarning("Couldn't open header file.");
    }

    QFile saveFileDeclaration(m_c_path+"/"+fileNameDeclaration);
    if (!saveFileDeclaration.open(QIODevice::WriteOnly)) {
       qWarning("Couldn't open c/cpp file.");
    }


    QVector<struct input_arguments> arguments;
    QVector<QString> memcpy_variable;
    QString function_name = "createUDPPackage";
    QString function_return_value = "void";
    QVector<QString> includes;
    includes.append("#include <string.h>\n");
    includes.append("#include <"+fileNameHeader.split("/")[fileNameHeader.split("/").length()-1]+">\n");

    QByteArray array;
    // Header ifndef ...
    QString tempFileNameHeader = function_name.toUpper()+"_H";
    array.append("#ifndef "+tempFileNameHeader+"\n");
    array.append("#define "+tempFileNameHeader+"\n");
    saveFileDefinition.write(array);

    struct input_arguments udp_buffer;
    udp_buffer.datatype = "char*";
    udp_buffer.variable_name = "udp_puffer";
    arguments.append(udp_buffer);

    getInputArguments(arguments);
    createMemcpyStrings(memcpy_variable);


    for(auto include: includes){
        saveFileDeclaration.write(include.toUtf8());
    }
    array.clear();
    array.append(function_return_value+" "+function_name);
    array.append("(");
    for (int i = 0; i<arguments.size(); i++){
        struct input_arguments argument = arguments[i];

        QString const_prefix;
        QString reference;
        if(i==0){
            const_prefix = "";
            reference = " ";
        }else{
            const_prefix = "const ";
            reference = "* ";
        }
        array.append(QString(const_prefix+argument.datatype+reference+argument.variable_name).toUtf8());
        if(i<arguments.size()-1){
            array.append(", ");
        }
    }
    array.append(")");
    saveFileDefinition.write(array+QString(";\n").toUtf8()); //write definition to file
    array.append("{\n");

    saveFileDeclaration.write(array);

    for(auto memcpy: memcpy_variable){
        saveFileDeclaration.write(QString("\t"+memcpy).toUtf8());
    }

   saveFileDeclaration.write("}");
   saveFileDeclaration.close();

   // Header File endif
   array.clear();
   array.append("#endif //"+ tempFileNameHeader);
   saveFileDefinition.write(array);
   saveFileDefinition.close();


}

void Signals::createMemcpyStrings(QVector<QString>& memcpy_strings){
    QString prefix="memcpy(";
    QString postfix = ");\n";

    memcpy_strings.append("char* pointer = udp_buffer\n\n");

    QString memcpy;
    QString pointer;

    for (auto signal : m_signals){
        pointer = "pointer += " + QString::number(signal.offset)+";\n";
        memcpy_strings.append(pointer);
        if(!isStruct(signal.name)){
            memcpy = prefix+"pointer, "+ signal.name+", sizeof(*"+signal.name+")"+postfix;
        }else{
            QString temp = signal.name;
            temp.replace(temp.indexOf("."),1,"->");
            memcpy = prefix+"pointer, &"+ temp+", sizeof("+temp+")"+postfix;
        }
        memcpy_strings.append(memcpy);
    }
}

bool Signals::isStruct(QString variable_name){
    if(variable_name.split(".").size()>1){
        return true;
    }
    return false;
}

bool Signals::ifVariableNameExist(const QVector<input_arguments>& arguments, QString variable_name, bool& ifstruct){
    ifstruct = isStruct(variable_name);
    for (auto argument : arguments){
        if (argument.variable_name.compare(variable_name.split(".")[0])== 0){
            return 1;
        }
    }
    return 0;
}

void Signals::getInputArguments(QVector<struct input_arguments>& arguments){
    // are variable with . in it's name --> struct
   struct input_arguments argument;
    for(auto signal : m_signals){
        bool ifstruct;
        if(ifVariableNameExist(arguments, signal.name, ifstruct)){
            continue;
        }

        if(ifstruct){
            argument.datatype = "struct";

        }else{
            argument.datatype = signal.datatype;
        }
        argument.variable_name = signal.name.split(".")[0]; // first is the highest struct name
        arguments.append(argument);
    }
}

void Signals::changeRelativeHeaderPath(QString relative_header_path){
    m_header_path = relative_header_path;
}
