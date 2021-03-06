#ifndef CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H
#define CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H

#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "plugins/EventManager.hpp"
#include "filehandler/filehandler.hpp"

namespace clas_digital
{
  class IReference
  {
    public:
      virtual const nlohmann::json &json() = 0;
      virtual IReference *json(nlohmann::json js) = 0;
      virtual bool IsAuthorEditor(std::string creator_type) = 0;
      virtual IReference *Copy() = 0;
      virtual std::string GetAuthor() = 0; 
      virtual std::string GetShortTitle() = 0;
      virtual std::string GetTitle() = 0;
      virtual std::string GetCitation() = 0;
      virtual int GetDate() = 0;
      virtual bool HasCopyright() = 0;
      virtual std::string GetKey() = 0;
      virtual std::string GetPath() = 0;
      virtual ~IReference(){}
      virtual std::string GetBibliography() = 0;
      virtual std::vector<std::string> GetCollections() = 0;
      virtual std::string GetShow2() = 0;
      virtual std::string GetName()  = 0;
      virtual bool HasContent() = 0;
      virtual bool HasOcr() = 0;
      virtual bool MoveTo(std::filesystem::path new_root) = 0;
      virtual std::vector<std::map<std::string,std::string>> GetAuthorKeys() = 0;

    protected:
  };




  /**
   * @brief Manages the reference information, for the moment only manages the
   * data from zotero but could be extended in a later version
   */
  class IReferenceManager
  {
    public:
      using container_t = std::unordered_map<std::string,IReference*>;
      using ptr_t = std::shared_ptr<IReference>;
      using ptr_cont_t = std::shared_ptr<container_t>;

      

      enum CacheOptions
      {
        CACHE_USE_CACHED,
        CACHE_FORCE_FETCH,
        CACHE_FAIL_ON_CACHE_MISS
      };

      enum Error
      {
        OK,
        CACHE_MISS,
        MULTIPLE_RESULTS,
        NO_JSON_RETURNED,
        KEY_DOES_NOT_EXIST,
        NO_API_KEY,
        NO_GROUP_OR_USER_ID,
        USER_ID_AND_GROUP_ID,
        JSON_FILE_DOES_NOT_EXIST,
        NOT_A_VALID_JSON,
        API_KEY_NOT_VALID_OR_NO_ACCESS,
        CACHE_FILE_PATH_NOT_VALID,
        NOT_MODIFIED,
        SERVER_SHUTDOWN_INTERRUPT,
        UNKNOWN
      };

      void SetEventManager(EventManager *manager)
      {
        event_manager_ = manager;
      }

      IReferenceManager(EventManager *usedManager) : event_manager_(usedManager) {}

      virtual Error GetAllItems(ptr_cont_t &items, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
      virtual Error GetAllCollections(ptr_cont_t &collections, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;

      virtual Error GetItemMetadata(ptr_t &item, std::string itemKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;
      virtual Error GetCollectionMetadata(ptr_t &collection, std::string collectionKey, CacheOptions opts=CacheOptions::CACHE_USE_CACHED) = 0;

      virtual Error SaveToFile() = 0;
      virtual ~IReferenceManager(){}
    protected:
      EventManager *event_manager_;
  };
}

#endif
