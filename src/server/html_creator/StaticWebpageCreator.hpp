#include "books/CBookManager.hpp"
#include "json.hpp"
#include <filesystem>
#include <inja/inja.hpp>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <mutex>

inline std::mutex global_lock_write;
static void atomic_write_file(std::string filename, const std::string &data)
{
	global_lock_write.lock();

	std::cout<<"Filename : "<<filename<<std::endl;
	
	std::ofstream ofs(filename.c_str(),std::ios::out);
	ofs<<data;
	ofs.close();

	global_lock_write.unlock();
}

class StaticWebpageCreator
{
	private:
		std::string m_path;
		std::string m_bookID;
		nlohmann::json m_info;
        nlohmann::json m_topnav;
		CBook *m_book;
	public:
        StaticWebpageCreator()
        {
            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
	    time_t now;
	    std::time(&now);
	    char buf[sizeof("YYYY-mm-ddTHH:MM:ssZ")];
	    std::strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
            m_topnav["time"] = buf;
	    char buf_human[sizeof("YYYY-mm-dd HH:MM:ss")];
	    strftime(buf_human, sizeof(buf_human), "%F %T", gmtime(&now));
	    m_topnav["time_human"] = buf_human;
        }

		StaticWebpageCreator(CBook *book)
		{
			m_path = book->get_path();
			m_bookID = book->get_key();
			m_info = book->get_metadata().get_json();
			m_book = book;

            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
	    time_t now;
	    time(&now);
	    char buf[sizeof("YYYY-mm-ddTHH:MM:ssZ")];
	    strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
            m_topnav["time"] = buf;
	    char buf_human[sizeof("YYYY-mm-dd HH:MM:ss")];
	    strftime(buf_human, sizeof(buf_human), "%F %T", gmtime(&now));
	    m_topnav["time_human"] = buf_human;
		}

		template<typename T>
		std::string general_applier(const std::string &esc, char character, T t1)
		{
			std::string newstr;
			for(unsigned int i = 0; i < esc.length(); i++)
				if( esc[i] == character )
				{
					newstr+=t1(character);
				}
				else
					newstr+=esc[i];
			return newstr;
		}
		std::string escaper(const std::string &esc,char character)
		{
			return general_applier(esc,character,[](char ch){return std::string("\\")+ch;});
		}

		std::string cutter(const std::string &esc, char character)
		{
			return general_applier(esc,character,[](char ){return "";});
		}

		std::string replacer(const std::string &esc, char character,std::string seq)
		{
			return general_applier(esc,character,[&seq](char){return seq;});
		}
		
		std::string escape_characters(std::string str)
		{
			return replacer(replacer(replacer(replacer(str,'<',"&lt;"),'>',"&gt;"),'"',"&quot;"),'\'',"&apos;");
		}

        void createSearchPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["search"] = "class='dropdown-banner active'";

            inja::Template temp = env.parse_template("web/search_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/Search.html", result);
        }

        void createInformationPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["information"] = "class='dropdown-banner active'";

            inja::Template temp = env.parse_template("web/information/information_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/information/index.de.html", result);
        }

        void createAdminPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["admin"] = "classifiedContent active";

            inja::Template temp = env.parse_template("web/private/admin/administration_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/private/admin/Administration.html", result);
        }

        void createUploadPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["upload"] = "classifiedContent active";

            inja::Template temp = env.parse_template("web/private/write/uploadBook_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/private/write/UploadBook.html", result);
        }

		void CreateMetadataPage(nlohmann::json pillars)
		{
			std::cout<<"Processing: "<<m_book->get_key()<<std::endl;
			auto itemType =m_info["data"].value("itemType","");
			auto title=m_info["data"].value("title","");
			auto isbn = m_info["data"].value("isbn","");
			nlohmann::json info;
            		info["show"] = m_book->get_metadata().GetShow2();
			info["bib"] = m_info["bib"].get<std::string>();
			info["bib_own"] = m_book->get_metadata().GetBibliographyEscaped();
			info["hasContent"] = m_book->hasContent();
			info["itemType"] = itemType;
			info["title"] = title;
			info["authors"] = nlohmann::json::array();
      for(auto author : m_book->get_metadata().GetAuthorsKeys())
      {
          nlohmann::json j;
          j["key"] = author["key"];
          j["name"] = author["fullname"];
          j["type"] = author["creator_type"];
          j["isAuthor"] = m_book->get_metadata().IsAuthorEditor(j["type"]);
  info["authors"].push_back(j);
      } 
			info["hasISBN"] = isbn != "";
			info["isbn"] = isbn;
			info["place"] = m_info["data"].value("place","");
			info["date"] = "";
            if(m_book->get_date() != -1)
                 info["date"] = m_book->get_date(); 
            info["hasDate"] = m_book->get_date() != -1;
			info["key"] = m_book->get_key();
            
            info["collections"] = nlohmann::json::array();
            for(const auto& it : m_book->get_metadata().GetCollections())
            {
                nlohmann::json j;
                j["key"] = it;
                j["name"] = "Untracked collection.";
                bool found = false;
                for(const auto& jt : pillars) {
                    if(jt["key"] == it)
                    {
                        found = true;
                        j["name"] = jt["name"];
                    }
                }
                if(found != false)
                    info["collections"].push_back(j);
            }

            //Parse navbar
            info["topnav"] = m_topnav;
            info["topnav"]["catalogue"] = "class='dropdown-banner active'";

			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/metadata_template.html");
			std::string result = env.render(temp, info);

			atomic_write_file("web/books/"+m_book->get_key()+"/index.html",result);
		}

		void CreatePagesPage()
		{
			nlohmann::json js;
		    
			std::error_code ec;
			std::filesystem::create_directory("web/books/"+m_book->get_key()+"/pages", ec);
		    //Parse navbar
		    js["topnav"] = m_topnav;
		    js["topnav"]["catalogue"] = "class='dropdown-banner active'";

			js["key"] = m_book->get_key();
			js["title"] = m_book->get_metadata().GetShow2(false);
			js["bib"] = m_info["bib"].get<std::string>();
			js["bib_esc"] = cutter(replacer(m_info["bib"].get<std::string>(),'"',"&quot;"),'\n');
			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/pages_template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/books/"+m_book->get_key()+"/pages/index.html",result);
		}
		bool createWebpage(nlohmann::json pillars)
		{
			std::string webpath = "/books/"+m_bookID+"/pages";
			std::error_code ec;
			
			//Remove the old folder structure
			//std::filesystem::remove_all(m_path+"/view",ec);
			//std::filesystem::remove_all(m_path+"/meta",ec);
			if(std::filesystem::exists(m_path+"/ocr.txt"))
			{
				CreatePagesPage();
			}
			

			{
				CreateMetadataPage(pillars);
			}
			return true;
		}
};

class StaticCatalogueCreator
{
    private: 
        nlohmann::json m_topnav;

	public:
        StaticCatalogueCreator()
        {
            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
	    time_t now;
	    time(&now);
	    char buf[sizeof("YYYY-mm-ddTHH:MM:ssZ")];
	    strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
            m_topnav["time"] = buf;   
	    char buf_human[sizeof("YYYY-mm-dd HH:MM:ss")];
	    strftime(buf_human, sizeof(buf_human), "%F %T", gmtime(&now));
	    m_topnav["time_human"] = buf_human;
        }

		void CreateCatalogue()
		{
			nlohmann::json js;
			js["pages"].push_back("books");
			js["pages"].push_back("authors");
			js["pages"].push_back("collections");
			js["pages"].push_back("years");
			js["world"] = "hallo";

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active'";


			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/catalogue/index.html",result);
		}

		void CreateCatlogueYears(CBookManager &mng)
		{
			nlohmann::json rend;

            //Parse navbar
            rend["topnav"] = m_topnav;
            rend["topnav"]["catalogue"] = "class='dropdown-banner active'";

			std::map<std::string,std::pair<int,std::list<CBook*>>> _map;
			for(auto &book : mng.getMapOfBooks())
			{
				std::string name = std::to_string(book.second->get_date());
				if(name=="-1")
					name="unknown";

				if(_map.count(name) > 0 )
					_map[name].first++;
				else
					_map[name].first=1;
				_map[name].second.push_back(book.second);
			}
			for(auto &x : _map)
			{
				nlohmann::json ks;
				ks["year"] = x.first;
				ks["count"] = x.second.first;
				rend["years"].push_back(ks);
			}
            
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/years/template.html");
			std::string result = env.render(temp, rend);
			atomic_write_file("web/catalogue/years/index.html",result);

			inja::Environment env2;
			inja::Template temp2 = env2.parse_template("web/catalogue/years/template_year.html");
			for(auto &x : _map)
			{
				std::error_code ec;
				std::filesystem::create_directory("web/catalogue/years/"+x.first,ec);
				nlohmann::json js;

                //Parse navbar
                js["topnav"] = m_topnav;
                js["topnav"]["catalogue"] = "class='dropdown-banner active'";

                std::vector<nlohmann::json> vBooks;
				for(auto y : x.second.second)
				{
					nlohmann::json js_k;
					js_k["key"] = y->get_metadata().GetMetadata("key", "data");
					js_k["title"] = y->get_metadata().GetShow2();
					js_k["bib"] = y->get_metadata().GetMetadata("bib");
					vBooks.push_back(std::move(js_k));
				}

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);
                js["books"] = vBooks;
				js["year"] = x.first;

				std::string result = env2.render(temp2, js);
				atomic_write_file("web/catalogue/years/"+x.first+"/index.html",result);
			}
		}

		void CreateCatalogueBooks(CBookManager &mng)
		{
            std::vector<nlohmann::json> vBooks;
			for(auto &it : mng.getMapOfBooks())
			{
				nlohmann::json entry;
				entry["key"] = it.second->get_metadata().GetMetadata("key", "data");
				entry["title"] = it.second->get_metadata().GetShow2();
				entry["bib"] = it.second->get_metadata().GetMetadata("bib");
				entry["has_ocr"] = it.second->has_ocr();
            
				vBooks.push_back(std::move(entry));
			}

            std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

			nlohmann::json books;
            books["books"] = vBooks;

            //Parse navbar
            books["topnav"] = m_topnav;
            books["topnav"]["catalogue"] = "class='dropdown-banner active'";

			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/books/template.html");
			std::string result = env.render(temp, books);
			atomic_write_file("web/catalogue/books/index.html",result);
		}

        void CreateCatalogueAuthors(CBookManager& manager)
		{
			nlohmann::json js;

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active'";

            //Using map to erase dublicates
            std::map<std::string, nlohmann::json> mapAuthors;
            for(auto &it : manager.getMapOfBooks()) 
            {
                for(auto author : it.second->get_metadata().GetAuthorsKeys())
                {
                    if(!it.second->get_metadata().IsAuthorEditor(author["creator_type"]))
                        continue;

                    mapAuthors[author["key"]] = {
                        {"id", author["key"]},
                        {"show", author["fullname"]},
                        {"num", manager.getMapofUniqueAuthors()[author["key"]].size()} 
                    };
                }
            }

            
            //Convert to vector, sort and add to json
            std::vector<nlohmann::json> vAuthors;
            for(auto &it : mapAuthors)
                vAuthors.push_back(it.second);
            std::sort(vAuthors.begin(), vAuthors.end(), &StaticCatalogueCreator::mySort);
            js["authors"] = vAuthors;
            

			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/authors/template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/catalogue/authors/index.html",result);
		}

        void CreateCatalogueAuthor(CBookManager& manager)
        {
            inja::Environment env;
            inja::Template temp = env.parse_template("web/catalogue/authors/template_author.html");

            for(const auto &it : manager.getMapOfBooks()) {

                for(auto author : it.second->get_metadata().GetAuthorsKeys())
                {
                    std::string key = author["key"];

                    if(author["lastname"].size() == 0)
                        continue;

                    if(!it.second->get_metadata().IsAuthorEditor(author["creator_type"]))
                        continue;

                    //Create directory
                    std::error_code ec;
                    std::filesystem::create_directory("web/catalogue/authors/"+key+"/", ec);

                    nlohmann::json js;
                    js["author"] = {{"name", author["fullname"]}, {"id", key}};

                    //Parse navbar
                    js["topnav"] = m_topnav;
                    js["topnav"]["catalogue"] = "class='dropdown-banner active'";

                    //Create json with all books
                    std::vector<nlohmann::json> vBooks;
                    if(manager.getMapofUniqueAuthors().count(key) == 0)
                        continue;

                    for(auto &jt : manager.getMapofUniqueAuthors()[key]) {
                        vBooks.push_back({ 
                            {"id", jt},
                            { "title", manager.getMapOfBooks()[jt]->get_metadata().GetShow2()},
                            { "bib", manager.getMapOfBooks()[jt]->get_metadata().GetMetadata("bib")},
                { "has_ocr", manager.getMapOfBooks()[jt]->has_ocr()}
                            });
                    }

                    std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

                    js["books"] = vBooks;
                    std::string result = env.render(temp, js);
                    atomic_write_file("web/catalogue/authors/"+key+"/index.html",result);
                }
            }
        }
    
        void CreateCatalogueCollections(nlohmann::json pillars)
        {
			nlohmann::json js;

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active'";

            for(auto &it : pillars)
                js["pillars"].push_back(it);
            
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/collections/template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/catalogue/collections/index.html",result);
		}

        void CreateCatalogueCollection(CBookManager& manager, nlohmann::json pillars)
        {
            inja::Environment env;
            inja::Template temp = env.parse_template("web/catalogue/collections/template_collection.html");
            for(auto& it : pillars)
            {
                nlohmann::json js;

                //Parse navbar
                js["topnav"] = m_topnav;
                js["topnav"]["catalogue"] = "class='dropdown-banner active'";

                js["pillar"] = it;
                std::string key = it["key"];

                //Create directory
                std::error_code ec;
                std::filesystem::create_directory("web/catalogue/collections/"+key+"/", ec);

                //Create books for this collection
                std::vector<nlohmann::json> vBooks;
                for(auto &jt : manager.getMapOfBooks()) {
                    std::vector<std::string> collections = jt.second->get_metadata().GetCollections();

                    if(collections.size() == 0 || func::in(key, collections) == false)
                        continue;

                    vBooks.push_back({ 
                        {"id", jt.first},
                        { "title", jt.second->get_metadata().GetShow2()},
                        { "bib", jt.second->get_metadata().GetMetadata("bib")},
			{"has_ocr",jt.second->has_ocr()}
			});
                }

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

                js["books"] = vBooks;
                std::string result = env.render(temp, js);
                atomic_write_file("web/catalogue/collections/"+key+"/index.html",result);
            }
        }

        static bool mySort(nlohmann::json i, nlohmann::json j) {
            std::string check = "";
            if(i.count("title") > 0) check = "title";
            else if(i.count("show") > 0) check = "show";
            else return true;

            std::string str1 = i[check];
            std::string str2 = j[check];
            return func::returnToLower(str1)<func::returnToLower(str2);
        }
};