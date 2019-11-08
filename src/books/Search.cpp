#include "CSearch.hpp"

/**
* @brief constructor
*/
CSearch::CSearch(CSearchOptions* searchOpts, std::string sWord) {
    m_sOpts = searchOpts;
    m_mapSR = new std::map<std::string, double>;
    m_sWord = sWord;
}

CSearch::~CSearch() {
    delete m_mapSR;
}

// *** GETTER *** //


/**
* @return searchOptions
*/
CSearchOptions* CSearch::getSearchOptions() {
    return m_sOpts;
}

std::string CSearch::getSearchedWord() {
    return m_sWord;
}
    

// *** GETTER (from searchoptions) *** //

/** 
* @return fuzzynes
*/
bool CSearch::getFuzzyness() {
    return m_sOpts->getFuzzyness();
}

/** 
* @return onlyTitle
*/
int CSearch::getOnlyTitle() {
    return m_sOpts->getOnlyTitle();
}

/** 
* @return onlyOcr
*/
int CSearch::getOnlyOcr() {
    return m_sOpts->getOnlyOcr();
}
// *** SETTER *** //

/**
* param[in] searchedWord set searched word
*/
void CSearch::setWord(std::string sWord) {
    m_sWord = sWord;
}


/**
* @brief calls spezific search function, searches, and creates map of  matches. Removes all 
* books that do not match with search options.
*/
std::map<std::string, double>* CSearch::search(MAPWORDS& mWs, MAPWORDS& mWsTitle, MAPWORDS& mWsAuthor, std::unordered_map<std::string, CBook*>& mapBooks)
{
    //Normal search (full-match)
    if (getFuzzyness() == false)
    {
        //Search in ocr and/ or in title
        if(getOnlyTitle() == false)
            normalSearch(mWs);
        if(getOnlyOcr() == false)
            normalSearch(mWsTitle);
        normalSearch(mWsAuthor);
    }

    //Fuzzy Search
    else
    {
        //Search in ocr and/ or in title
        if(getOnlyTitle() == false)
            fuzzySearch(mWs, mapBooks, false);
        if(getOnlyOcr() == false)
            fuzzySearch(mWsTitle, mapBooks, true);
        fuzzySearch(mWsAuthor, mapBooks, true);
    }


    //Check search-options and remove books from search results, that don't match
    removeBooks(mapBooks);

    return m_mapSR;
}

/**
* @brief search full-match
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::normalSearch(MAPWORDS& mapWords)
{
    std::cout << "Searching for " << m_sWord << "\n";
    if(mapWords.count(m_sWord) > 0) {
        std::map<std::string, double> found = mapWords.at(m_sWord);
        for(auto jt=found.begin(); jt!=found.end(); jt++)
                (*m_mapSR)[jt->first] += jt->second;

    }
}


/**
* @brief search fuzzy 
* @param[in] mapWords map of all words with a list of books in which this word accures
* @param[in, out] mapSR searchresults
*/
void CSearch::fuzzySearch(MAPWORDS& mapWords, std::unordered_map<std::string, CBook*>& mapBooks, bool t)
{
    for(auto it= mapWords.begin(); it!=mapWords.end(); it++)
    {
        double value = fuzzy::fuzzy_cmp(it->first, m_sWord);
        if(value <= 0.2 && t==false) 
            myInsert(it->second, it->first, mapBooks, value);
        else if(value <= 0.2)
        {
            std::map<std::string, double> found = it->second;
            for(auto jt=found.begin(); jt!=found.end(); jt++)
                (*m_mapSR)[jt->first] += jt->second;
        }
    }
}

/*
* @brief inserts searchResults into map of searchresults and assigns value of match
* @param[out] mapSR
* @param[in] found
* @param[out] sMatch
* @param[in] value
*/
void CSearch::myInsert(std::map<std::string, double>& found, std::string sMatch, std::unordered_map<std::string, CBook*>& mapBooks, double value)
{
    for(auto it=found.begin(); it!=found.end(); it++) {
        (*m_mapSR)[it->first] += it->second*(1-value*5);

        //Add match to map
        if(mapBooks[it->first]->getOcr() == false)
            continue;
        if (mapBooks[it->first]->getMapFuzzy()[m_sWord].front().second > value)
            mapBooks[it->first]->getMapFuzzy()[m_sWord].push_front({sMatch, value});
        else
            mapBooks[it->first]->getMapFuzzy()[m_sWord].push_back({sMatch, value});
    }
}

/**
* @brief remove all books that don't agree with searchOptions.
* @param[in, out] mapSR map of search results
*/
void CSearch::removeBooks(std::unordered_map<std::string, CBook*>& mapBooks)
{
    for(auto it=m_mapSR->begin(); it!=m_mapSR->end();)
    {
        if(checkSearchOptions(mapBooks[it->first]) == false)
            m_mapSR->erase(it++);
        else
            ++it;
    }
}

/**
* @param[in] book to be checked
* return Boolean
*/
bool CSearch::checkSearchOptions(CBook* book)
{
    //*** check ocr ***//
    if(m_sOpts->getOnlyOcr() == true && book->getOcr() == false)
        return false;

    //*** check author ***//
    if(m_sOpts->getLastName().length() > 0)
    {
        if(book->getAuthor() != m_sOpts->getLastName())
            return false;
    }

    //*** check date ***//
    if(book->getDate()==-1 || book->getDate()<m_sOpts->getFrom() || book->getDate()>m_sOpts->getTo())
        return false;
         
    //*** check pillars ***//
    for(auto const &collection : m_sOpts->getCollections()) {
        if(func::in(collection, book->getMetadata().getCollections()) == true)
            return true;
    }
    return false;
}




