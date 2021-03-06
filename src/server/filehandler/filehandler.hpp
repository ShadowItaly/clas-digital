#ifndef CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#define CLASDIGITAL_SRC_SERVER_FILEHANDLER_H
#include <filesystem>
#include <atomic>
#include <unordered_map>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_THREAD_POOL_COUNT 8
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#endif

#include <shared_mutex>
#include <fstream>
#include <cache/cache.h>
#include <login/user.hpp>

namespace clas_digital
{

  static unsigned long long GetFreeSpace(std::filesystem::path path) {
    std::error_code ec;
    std::filesystem::space_info tmp = std::filesystem::space(path,ec);
    if(!ec) {
      return tmp.free;
    }
    else
      return -1;
  }
 
  class IFileHandler
  {
    public:
      virtual void AddMountPoint(std::filesystem::path pt) = 0;
      virtual void AddUploadPoint(std::filesystem::path pt) = 0;
      virtual void ServeFile(const httplib::Request &req, httplib::Response &resp, bool abortoncachemiss=false) = 0;
      virtual void AddAlias(std::vector<std::string> from, std::filesystem::path to) = 0;
      virtual std::vector<std::filesystem::path> &GetMountPoints() = 0;
      virtual std::vector<std::filesystem::path> &GetUploadPoints() = 0;
      virtual std::filesystem::path GetFreestMountPoint() = 0;
  };

  class FileHandler : public IFileHandler
  {
    public:
      using cache_t = std::shared_ptr<std::vector<char>>;

      FileHandler(long long cache_size);
      virtual void AddMountPoint(std::filesystem::path pt) override;
      virtual void ServeFile(const httplib::Request &req, httplib::Response &resp, bool abortoncachemiss=false) override;
      virtual void AddAlias(std::vector<std::string> from, std::filesystem::path to) override;

      std::function<bool(const std::filesystem::path&)> cache_file_callback_;
      void AddUploadPoint(std::filesystem::path pt) {
        upload_points_.push_back(pt);
      }

      std::vector<std::filesystem::path> &GetMountPoints() {
        return mount_points_;
      }

      std::vector<std::filesystem::path> &GetUploadPoints() {
        return upload_points_;
      }
      
      std::filesystem::path GetFreestMountPoint() override {
        unsigned long long maxSpace = 0;
        std::filesystem::path maxPath;
        std::error_code ec;
        for(auto &it : upload_points_) {
          std::cout<<"Processing: "<<it<<std::endl;
          std::filesystem::space_info tmp = std::filesystem::space(it,ec);
          if(!ec && maxSpace <= tmp.free) {
            maxPath = it;
            maxSpace = tmp.free;
          }
        }
        return maxPath;
      }
    private:
      FixedSizeCache<std::string> cache_;
      std::vector<std::filesystem::path> mount_points_;
      std::vector<std::filesystem::path> upload_points_;
      std::map<std::string,std::string> file_types_;



      std::string __getFileMimetype(const std::filesystem::path &mime);
  };
}


#endif
