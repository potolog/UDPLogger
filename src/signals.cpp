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
    parseJsonObject(object);
}

void Signals::writeToJsonObject(QJsonObject &object){
    QJsonArray active_signals;

    for(auto signal : m_signals){
        QJsonObject signal_object;
        signal_object["Datatype"] = signal.datatype;
        signal_object["Index"] = signal.index;
        signal_object["Offset"]  = signal.offset;
        signal_object["Signalname"]  = signal.name;
        signal_object["structName"] = signal.struct_name;
        active_signals.append(signal_object);
    }
    object["Signals"] = active_signals;
}

void Signals::parseJsonObject(QJsonObject &object){
    if(object.contains("Signals")){
        QJsonArray imported_signals = object["Signals"].toArray();
        QVector<struct Signal> signal_vector;
        struct Signal signal_element;

        bool success = true;
        for(int i=0; i<imported_signals.count(); i++){
            QJsonObject signal = imported_signals[i].toObject();
            signal_element.datatype = validateDatatypes(signal["Datatype"].toString(),success);
            signal_element.index = signal["Index"].toInt();
            signal_element.offset = signal["Offset"].toInt();
            signal_element.name = signal["Signalname"].toString();
            signal_element.struct_name = signal["structName"].toString();
            signal_vector.append(signal_element);
            if(signal_element.datatype.compare("")==0){
                success = false;
                emit showMessageBox(tr("No valid datatype"),QString(tr("The datatype of '%1' (%2) is not valid! \n No signals changed.")).arg(signal_element.name).arg(signal_element.datatype));
                break;
            }

        }
        if(success){
            setSignals(&signal_vector);
        }
    }
}

int Signals::calculateDatatypeSize(QString datatype){
    if(datatype.compare("int8_t")==0){
        return 1;
    }
    if(datatype.compare("uint8_t")==0){
        return 1;
    }
    if(datatype.compare("bool")==0){
        return 1;
    }
    if(datatype.compare("int16_t")==0){
        return 2;
    }
    if(datatype.compare("uint16_t")==0){
        return 2;
    }
    if(datatype.compare("int32_t")==0){
        return 4;
    }
    if(datatype.compare("uint32_t")==0){
        return 4;
    }
    if(datatype.compare("float")==0){
        return 4;
    }
    if(datatype.compare("double")==0){
        return 8;
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
     QString index_column_string = "Index";
     for (unsigned long i=0; i < theWholeSpreadSheet.size(); i++){
         auto row = theWholeSpreadSheet.at(i);
         QString column_null = QString::fromStdString(row[0]);
         if(column_null.compare(index_column_string)==0){
            description_row = i;
         }
     }

     if(description_row <0){
        showMessageBox(tr("No description row found"), tr("No row found, where in the first column is the text '%1'.").arg("Index"));
        return -1;
     }

     // Find columns where
     long datatype_column = -1;
     QString datatype_column_string = "Datatype";
     long index_column = 0;
     long name_column = -1;
     QString name_column_string = "Signalname";
     long struct_name_column = -1;
     QString struct_name_column_string = "Structname";
     long unit_name_column = -1;
     QString unit_name_column_string = "Unit";

     for(unsigned long column=0; column < theWholeSpreadSheet.at(description_row).size(); column++){
         QString column_value = QString::fromStdString(theWholeSpreadSheet.at(description_row).at(column));
         if(column_value.compare(datatype_column_string)== 0){
             datatype_column = column;
             continue;
         }

         if(column_value.compare(name_column_string)==0){
             name_column = column;
             continue;
         }

         if(column_value.compare(struct_name_column_string)==0){
             struct_name_column = column;
             continue;
         }
         if(column_value.compare(unit_name_column_string)==0){
             unit_name_column = column;
             continue;
         }
     }

     if(datatype_column < 0){
         showMessageBox(tr("Column not found"),tr("No column found with the name '%1'. Make sure, that this column name is in the same row as the Index description").arg(datatype_column_string));
         return -2;
     }else if(index_column < 0){
         showMessageBox(tr("Column not found"),tr("No column found with the name '%1'. Make sure, that this column name is in the same row as the Index description").arg(index_column_string));
         return -2;
     }else if(name_column < 0){
         showMessageBox(tr("Column not found"),tr("No column found with the name '%1'. Make sure, that this column name is in the same row as the Index description").arg(name_column_string));
         return -2;
     }else if(struct_name_column < 0){
         showMessageBox(tr("Column not found"),tr("No column found with the name '%1'. Make sure, that this column name is in the same row as the Index description").arg(struct_name_column_string));
         return -2;
     }else if(unit_name_column < 0){
         showMessageBox(tr("Column not found"),tr("No column found with the name '%1'. Make sure, that this column name is in the same row as the Index description").arg(unit_name_column_string));
         return -2;
     }

     int offset = 0;
     bool success = true;
     for(unsigned long i=description_row+1; i< theWholeSpreadSheet.size(); i++){
         struct Signal signal;
         signal.datatype = validateDatatypes(QString::fromStdString(theWholeSpreadSheet.at(i).at(datatype_column)),success);
         if(!success){
             success = false;
             emit showMessageBox(tr("No valid datatype"),QString(tr("The datatype of '%1' (%2) is not valid! \n No signals changed.")).arg(signal.name).arg(signal.datatype));
             break;
         }
         signal.name = QString::fromStdString(theWholeSpreadSheet.at(i).at(name_column));
         signal.index = std::stoi(theWholeSpreadSheet.at(i).at(index_column));
         signal.offset = offset;
         signal.unit = QString::fromStdString(theWholeSpreadSheet.at(i).at(unit_name_column));
         signal.struct_name = QString::fromStdString(theWholeSpreadSheet.at(i).at(struct_name_column));
         offset+= calculateDatatypeSize(signal.datatype);
         new_signals.append(signal);

     }
     if(success){
        m_signals.clear();
        m_signals = new_signals;
        emit signalsChanged();
     }
    return 0;
}

bool Signals::signalExist(struct Signal signal_to_match) const{
    foreach(struct Signal signal, m_signals){
        bool datatype = signal.datatype.compare(signal_to_match.datatype)==0;
        bool name = signal.name.compare(signal_to_match.name)==0;
        bool index = signal.index == signal_to_match.index;
        bool unit = signal.unit == signal_to_match.unit;
        bool struct_name = signal.struct_name == signal_to_match.struct_name;
        bool offset = signal.offset == signal_to_match.offset;
        if(datatype && name && index && unit && struct_name && offset){
            return 1;
        }
    }
    return 0;

}

QString Signals::validateDatatypes(QString datatype, bool &success){
    success = true;
    if(datatype.compare("char")==0 || datatype.compare("int8_t")==0){
        return "int8_t";
    }
    if(datatype.compare("uint8_t")==0){
        return "uint8_t";
    }
    if(datatype.compare("bool")==0){
        return datatype;
    }
    if(datatype.compare("int16_t")==0){
        return datatype;
    }
    if(datatype.compare("uint16_t")==0){
        return "uint16_t";
    }
    if(datatype.compare("int")==0 || datatype.compare("int32_t")==0){
        return "int32_t";
    }
    if(datatype.compare("uint32_t")==0){
        return datatype;
    }
    if(datatype.compare("float")==0){
        return datatype;
    }
    if(datatype.compare("double")==0){
        return datatype;
    }

    success = false;
    return datatype; // not supported datatype detected
}

void Signals::importSignals(){
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open Signals file"), "/home", tr("SignalFiles(*.udpLoggerSignals, *.xlsx)"));
    if(fileName.compare("")==0)
        return;

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

    if(pathWithFileName.compare("")==0){
        return;
    }

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
    QString function_return_value = "int";
    QVector<QString> includes;
    includes.append("#include <string.h>\n");
    includes.append("#include <"+fileNameHeader.split("/")[fileNameHeader.split("/").length()-1]+">\n");
    if(!m_additional_includes.isEmpty()){
        QStringList additional_includes = m_additional_includes.split(";");
        foreach(QString include, additional_includes){
           includes.append("#include <"+include+">\n");
        }
    }

    QByteArray array;
    // Header ifndef ...
    QString tempFileNameHeader = function_name.toUpper()+"_H";
    array.append("#ifndef "+tempFileNameHeader+"\n");
    array.append("#define "+tempFileNameHeader+"\n");
    saveFileDefinition.write(array);
    saveFileDefinition.write("#include <stdint.h> \n");

    // create arguments list
    struct input_arguments udp_buffer;
    udp_buffer.datatype = "char* ";
    udp_buffer.variable_name = "udp_buffer";
    arguments.append(udp_buffer);
    struct input_arguments buffer_length;
    buffer_length.datatype = "int";
    buffer_length.variable_name = "buffer_length";
    arguments.append(buffer_length);
    getInputArguments(arguments); // creates string of input arguments

    QString buffersize_condition = ifConditionBuffersize(buffer_length);

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
        if(i==0){
            const_prefix = "";
        }else{
            const_prefix = "const ";
        }
        array.append(QString(const_prefix+argument.datatype+" "+argument.variable_name).toUtf8());
        if(i<arguments.size()-1){
            array.append(", ");
        }
    }
    array.append(")");
    saveFileDefinition.write(array+QString(";\n").toUtf8()); //write definition to file
    array.append("{\n");

    saveFileDeclaration.write(array);

    saveFileDeclaration.write(buffersize_condition.toUtf8());

    for(auto memcpy: memcpy_variable){
        saveFileDeclaration.write(QString("\t"+memcpy).toUtf8());
    }

   saveFileDeclaration.write("\treturn 0;\n"); // successfully packaged

   saveFileDeclaration.write("}");
   saveFileDeclaration.close();

   // Header File endif
   array.clear();
   array.append("#endif //"+ tempFileNameHeader);
   saveFileDefinition.write(array);
   saveFileDefinition.close();
}

QString Signals::ifConditionBuffersize(struct input_arguments buffer_length_variable){
    int length = calculateMinBufferLength(); // length in byte

    QString condition = "\tif("+QString::number(length) + ">" + buffer_length_variable.variable_name + "){\n";
    condition += "\t\t return -1;\n";
    condition += "\t}\n";
    return condition;
}

int Signals::calculateMinBufferLength(){
    struct Signal signal = m_signals.at(m_signals.length()-1);

    int offset = signal.offset + calculateDatatypeSize(signal.datatype);
    return offset;
}

void Signals::createMemcpyStrings(QVector<QString>& memcpy_strings){
    QString prefix="memcpy(";
    QString postfix = ");\n";

    memcpy_strings.append("char* pointer = udp_buffer;\n\n");

    QString memcpy;
    QString pointer;

    for (auto signal : m_signals){
        pointer = "pointer = udp_buffer + " + QString::number(signal.offset)+";\n";
        memcpy_strings.append(pointer);
        if(!isStruct(signal.name)){
            memcpy = prefix+"pointer, "+"&"+ signal.name+", sizeof("+signal.name+")"+postfix;
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

bool Signals::ifStructNameExist(const QVector<input_arguments>& arguments, QString struct_name, bool& ifstruct){
    for (auto argument : arguments){
        if (argument.datatype.compare("struct "+struct_name+"*")== 0){
            ifstruct = true;
            return 1;
        }
    }
    if(struct_name.compare("")==0){
        ifstruct = false;
        return 0;
    }
    ifstruct = true;
    return 0;
}

void Signals::getInputArguments(QVector<struct input_arguments>& arguments){
    // are variable with . in it's name --> struct
   struct input_arguments argument;
    for(auto signal : m_signals){
        bool ifstruct;
        if(ifStructNameExist(arguments, signal.struct_name, ifstruct)){
            continue;
        }

        if(ifstruct){
            argument.datatype = "struct "+signal.struct_name +"*";
        }else{
            argument.datatype = signal.datatype;
        }
        argument.variable_name = signal.name.split(".")[0]; // first is the highest struct name
        arguments.append(argument);
    }
}

void Signals::changeSignalSettings(QString relative_header_path, QString additional_includes){
    m_header_path = relative_header_path;
    m_additional_includes = additional_includes;
}
