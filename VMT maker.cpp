#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
unsigned int virtualFunctionCount = 0;

int getOffsetPos(const string &line)
{
    int position = line.find("dd offset");
    if (position == string::npos) return -1;
    else return position;
}

bool isSub(string& name)
{
    if (name.substr(0, 4) == "sub_") return true;
    return false;
}

bool isVirtual(string &name)
{
    if (name[0] == name[1] == '_' )
    {
        virtualFunctionCount++;
        return true;
    }
    return false;
}

string extractName(string line)
{
    int namePosition = line.find(";");  //erase leftover front
    if(namePosition!=string::npos)
        line.erase(0, namePosition + 2);
    int argsPosition = line.find("(");  //erase parentheses
    if(argsPosition!=string::npos)
        line.erase(argsPosition, line.size() - argsPosition);
    return line;
}

void onFirstLine(vector<string>&vec, ifstream &source, string &line)
{
    getline(source, line);//'dd offset' line
    int ddOffsetPos = getOffsetPos(line);
    line.erase(0, ddOffsetPos + 10);
    if (!isSub(line) && !isVirtual(line)) 
    {
        bool isName = false;
        while (!isName)
        {
            getline(source, line);
            if (line[line.size() - 1] != 'o')
            {
                isName = true;
                line = extractName(line);
            }
        }
    }
    vec.push_back("\tDWORD *" + line + ";");
}

vector<string> processInput(vector<string>&vec, ifstream &source)
{
    string buffer;
    int ddOffsetPos = 0;
    onFirstLine(vec, source, buffer);   //skip xrefs
    while (!source.eof())
    {
        getline(source, buffer);
        ddOffsetPos = getOffsetPos(buffer);
        if (ddOffsetPos != -1)
        {
            buffer.erase(0, ddOffsetPos + 10);          //erase front
            if (isVirtual(buffer))
                buffer = "p" + buffer + to_string(virtualFunctionCount);
            else if (!isSub(buffer))   //erase leftovers if named  
                buffer = extractName(buffer);
            vec.push_back("\tDWORD *" + buffer + ";");
        }
    }
    return vec;
}

int main()
{
    ifstream source;
    string tmp;
    vector<string> structDefinition;
    cout << "Full path to the .txt (remember of double backslashes):\n";
    getline(cin, tmp);
    source.open(tmp);
    cout << "Class name of the vmt ('::VMT' will be appended):\n";
    getline(cin, tmp);
    string structHeader = "struct " + tmp + "::VMT";
    structDefinition.push_back(structHeader);
    structDefinition.push_back("{");
    processInput(structDefinition, source);
    structDefinition.push_back("};");
    for (auto line : structDefinition)
        cout << line << endl;
    source.close();
    return 0;
}
