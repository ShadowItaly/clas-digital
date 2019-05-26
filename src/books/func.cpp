#include "func.hpp"

namespace func 
{

/*
* in: checks whether a string is in a vector of strings
* @parameter string
* @parameter vector<string> 
* @return bool
*/
bool in(std::string str, std::vector<std::string> vec)
{
    for(unsigned int i=0; i<vec.size(); i++)
    {
        if(compare(str, vec[i])== true)
            return true;
    }

    return false;
}

/**
* @brief expects words to be non-capital letters
* @param[in] chT1 first string to compare
* @param[in] chT2 second string to compare
* @return Boolean indicating, whether strings compare or not
*/
bool compare(const char* chT1, const char* chT2)
{
    //Return false, if length of words differs
    if(strlen(chT1) != strlen(chT2))
        return false;

    //Iterate over characters. Return false if characters don't match
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        if(chT1[i] != chT2[i])
            return false;
    }
    
    return true;
}

/**
* @brief takes to strings, converts to lower and calls compare (const char*, const char*)
* @param[in] str1 first string to compare
* @param[in] str2 string to compare first string with
* @return Boolean indicating, whether strings compare or not
*/
bool compare(std::string str1, std::string str2)
{
    //Convert both strings to lower
    convertToLower(str1);
    convertToLower(str2);

    //Call normal compare funktion
    return compare(str1.c_str(), str2.c_str());
}

/**
* @brief expects words to be non-capital letters
* @param[in] chT1 first string
* @param[in] chT2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(const char* chT1, const char* chT2)
{
    bool r = true;
    for(unsigned int i=0; i<strlen(chT1); i++)
    {
        r = true;
        for(unsigned int j=0; j<strlen(chT2); j++)
        {
            if(chT1[i+j] != chT2[j]) {
                r = false;
                break;
            }
        }

        if (r==true)
            return true;
    }

    return false;
}

/**
* @brief takes to strings, converts to lower and calls contains (const char*, const char*)
* @brief expects words to be non-capital letters
* @param[in] s1 first string
* @param[in] s2 second string, check whether it is in the first
* @return Boolean indicating, whether string1 contains string2 or not
*/
bool contains(std::string s1, std::string s2)
{
    //Convert both strings to lower
    convertToLower(s1);
    convertToLower(s2);

    //Call normal contains function
    return contains(s1.c_str(), s2.c_str());
}


/**
* @param[in, out] str string to be modified
*/
void convertToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    for(unsigned int i=0; i<str.length(); i++)
        str[i] = tolower(str[i], loc1);
}

/** 
* @brief function checks whether character is a letter with de and fr local
* @param[in] s char to be checked
*/
bool isLetter(const char s)
{
    std::locale loc1("de_DE.UTF8");
    //std::locale loc2("fr_FR");
    if(std::isalpha(s, loc1) == true) 
        return true;
    else
        return false;
}

/**
* @brief checks whether a string is a word
* @param[in] chWord string to be checked
* @return boolean for words/ no word
*/
bool isWord(const char* chWord) 
{
    //Count number of non-letters in string
    unsigned int counter = 0;
    for(unsigned int i=0; i<strlen(chWord); i++)
    {
        if(isLetter(chWord[i]) == false)
            counter++;
    }

    //Calculate whether more the 30% of characters are non-letters (return false)
    if(static_cast<double>(counter)/static_cast<double>(strlen(chWord)) <= 0.3)
        return true;

    return false;
}

/**
* @param[in] str string to be splitet
* @param[in] delimitter 
* @param[in, out] vStr vector of slpitted strings
*/
void split(std::string str, std::string delimiter, std::vector<std::string>& vStr)
{
    size_t pos=0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        vStr.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }

    vStr.push_back(str);
} 

/**
* @brief cuts all non-letter-characters from end and beginning of str
* @param[in, out] string to modify
*/
void transform(std::string& str)
{
    if(str.length() == 0)
        return;

    if(isLetter(str.front()) == false)
        transform(str.erase(0,1));

    if(str.length() == 0)
        return;

    if(isLetter(str.back()) == false)
    {
        str.pop_back();
        transform(str);
    }
}

/**
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
void extractWordsFromString(std::string sWords, std::map<std::string, int>& mapWords)
{
    std::vector<std::string> vStrs;
    split(sWords, " ", vStrs);

    for(unsigned int i=0; i<vStrs.size(); i++)
    {
        transform(vStrs[i]);
        if(isWord(vStrs[i].c_str()) == true)
        {
            convertToLower(vStrs[i]);
            mapWords[vStrs[i]] = 0;
        }
    }
}

/**
* @param[in] sWords string of which map shall be created 
* @param[out] mapWords map to which new words will be added
*/
void extractWordsFromString(std::string sWords, std::list<std::string>& listWords)
{
    if (checkPage(sWords) == true)
    {
        listWords.push_back("###page###");
        return;
    }

    std::vector<std::string> vStrs;
    split(sWords, " ", vStrs);

    for(unsigned int i=0; i<vStrs.size(); i++)
    {
        transform(vStrs[i]);
        if(isWord(vStrs[i].c_str()) == true)
        {
            convertToLower(vStrs[i]);
            listWords.push_back(vStrs[i]);
        }
    }
}

/**
* @param[in] sPathToWords path to .txt with all words in book
* @param[out] mapWords map to which new words will be added.
*/
void loadMapOfWords(std::string sPathToWords, std::map<std::string, int>& mapWords)
{
    std::ifstream read(sPathToWords);

    std::string sBuffer;
    while(!read.eof())
    {
        //Read new line
        getline(read, sBuffer);

        //Add word to map of words 
        mapWords[sBuffer] = 0;
    }
 
}

/**
* @brief check whether string indicates, that next page is reached
* @param[in] buffer 
* @return 
*/
bool checkPage(std::string &buffer)
{
    const char arr[] = "----- ";
    if(buffer.length()<6)
        return false;

    for(unsigned int i=0; i < 6;i++)
    {
        if(arr[i]!=buffer[i])
        return false;
    }

    for(unsigned int i = 6; i < buffer.length(); i++)
    {
        if(buffer[i]==' ')
            return true;
        else if(buffer[i]<48||buffer[i]>57)
            return false;
   }
   return false;
}


} //Close namespace 