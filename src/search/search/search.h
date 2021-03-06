/**
 * @author fux
*/

#ifndef CLASDIGITAL_SRC_SEARCH_SEARCH_SEARCH_H
#define CLASDIGITAL_SRC_SEARCH_SEARCH_SEARCH_H

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#include "book_manager/book.h"
#include "func.hpp"
#include "fuzzy.hpp"
#include "search/search_options.h"

class Search
{
private:
    std::string m_sWord;
    SearchOptions* m_sOpts;
    std::map<std::string, double>* m_mapSR;

    typedef std::unordered_map<std::string, std::map<std::string, double>> MAPWORDS;
    typedef std::unordered_map<std::string, Book*> map_books;
    typedef std::unordered_map<std::string, std::set<std::string>> dict;

public:
    
    /**
    * @brief constructor
    */
    Search(SearchOptions* searchOpts, std::string sWord);
    ~Search();

    // *** GETTER *** //


    /**
    * @return searchOptions
    */
    SearchOptions* getSearchOptions();

    /**
    * @return searched word
    */
    std::string getSearchedWord();


    // *** GETTER (from searchoptions) *** //

    /** 
    * @return fuzzynes
    */
    bool getFuzzyness();

    /** 
    * @return onlyTitle
    */
    int getOnlyTitle();

    /** 
    * @return onlyOcr
    */
    int getOnlyOcr();
    
    // *** SETTER *** //

    /**
    * @brief set searched word.
    * param[in] searchedWord set searched word
    */
    void setWord(std::string sWord);

    /**
    * @brief calls spezific search function, searches, and creates map of  matches. Removes all 
    * books that do not match with search options.
    */
    std::map<std::string, double>* search(MAPWORDS& mWs, MAPWORDS& mWsTitle, MAPWORDS& mWsAuthor,
                std::unordered_map<std::string, Book*>& mapBooks, dict& d_dict);

    /**
    * @brief search full-match
    * @param[in] mapWords map of all words with a list of books in which this where accures
    * @param[in, out] mapSR map of search results
    */
    void normalSearch(MAPWORDS& mapWords);
    void normalSearch(MAPWORDS& mapWords, dict& d_dict, map_books& mapBooks);

    /**
    * @brief search fuzzy 
    * @param[in] mapWords map of all words with a list of books in which this word accures
    * @param[in, out] mapSR searchresults
    */
    void fuzzySearch(MAPWORDS& mapWords, map_books& mapBooks, bool t);

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] matches
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, map_books& mapBooks, double value);

    /*
    * @brief inserts searchResults into map of searchresults and assigns value of match
    * @param[out] mapSR
    * @param[in] found
    * @param[out] sMatch
    * @param[in] value
    */
    void myInsert(std::map<std::string, double>& found, std::string sMatch, map_books& mapBooks);

    /**
    * @brief remove all books that do not match with searchoptions
    * @param[in, out] mapSR map of search results
    */
    void removeBooks(map_books& mapBooks);

    /**
    * @brief check whether book-metadata matches with searchoptions
    * @param[in] book to be checked
    * return Boolean
    */
    bool checkSearchOptions(Book* book);
};

#endif
