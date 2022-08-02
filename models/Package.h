/**
 *
 *  Package.h
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#pragma once
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/SqlBinder.h>
#include <drogon/orm/Mapper.h>
#include <BaseBuilder.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>
#include <json/json.h>
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <stdint.h>
#include <iostream>

namespace drogon
{
namespace orm
{
class DbClient;
using DbClientPtr = std::shared_ptr<DbClient>;
}
}
namespace drogon_model
{
namespace postgres
{

class Package
{
  public:
    struct Cols
    {
        static const std::string _id;
        static const std::string _published_at;
        static const std::string _metadata;
        static const std::string _name;
        static const std::string _version;
        static const std::string _edition;
        static const std::string _authors;
        static const std::string _repository;
        static const std::string _description;
        static const std::string _license;
        static const std::string _readme;
        static const std::string _sha256sum;
    };

    const static int primaryKeyNumber;
    const static std::string tableName;
    const static bool hasPrimaryKey;
    const static std::string primaryKeyName;
    using PrimaryKeyType = std::string;
    const PrimaryKeyType &getPrimaryKey() const;

    /**
     * @brief constructor
     * @param r One row of records in the SQL query result.
     * @param indexOffset Set the offset to -1 to access all columns by column names,
     * otherwise access all columns by offsets.
     * @note If the SQL is not a style of 'select * from table_name ...' (select all
     * columns by an asterisk), please set the offset to -1.
     */
    explicit Package(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     */
    explicit Package(const Json::Value &pJson) noexcept(false);

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     * @param pMasqueradingVector The aliases of table columns.
     */
    Package(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    Package() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false);
    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false);
    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForCreation(const Json::Value &,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                    std::string &err);
    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForUpdate(const Json::Value &,
                                          const std::vector<std::string> &pMasqueradingVector,
                                          std::string &err);
    static bool validJsonOfField(size_t index,
                          const std::string &fieldName,
                          const Json::Value &pJson,
                          std::string &err,
                          bool isForCreation);

    /**  For column id  */
    ///Get the value of the column id, returns the default value if the column is null
    const std::string &getValueOfId() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getId() const noexcept;
    ///Set the value of the column id
    void setId(const std::string &pId) noexcept;
    void setId(std::string &&pId) noexcept;

    /**  For column published_at  */
    ///Get the value of the column published_at, returns the default value if the column is null
    const ::trantor::Date &getValueOfPublishedAt() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<::trantor::Date> &getPublishedAt() const noexcept;
    ///Set the value of the column published_at
    void setPublishedAt(const ::trantor::Date &pPublishedAt) noexcept;

    /**  For column metadata  */
    ///Get the value of the column metadata, returns the default value if the column is null
    const std::string &getValueOfMetadata() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getMetadata() const noexcept;
    ///Set the value of the column metadata
    void setMetadata(const std::string &pMetadata) noexcept;
    void setMetadata(std::string &&pMetadata) noexcept;

    /**  For column name  */
    ///Get the value of the column name, returns the default value if the column is null
    const std::string &getValueOfName() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getName() const noexcept;
    ///Set the value of the column name
    void setName(const std::string &pName) noexcept;
    void setName(std::string &&pName) noexcept;

    /**  For column version  */
    ///Get the value of the column version, returns the default value if the column is null
    const std::string &getValueOfVersion() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getVersion() const noexcept;
    ///Set the value of the column version
    void setVersion(const std::string &pVersion) noexcept;
    void setVersion(std::string &&pVersion) noexcept;

    /**  For column edition  */
    ///Get the value of the column edition, returns the default value if the column is null
    const short &getValueOfEdition() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<short> &getEdition() const noexcept;
    ///Set the value of the column edition
    void setEdition(const short &pEdition) noexcept;

    /**  For column authors  */
    ///Get the value of the column authors, returns the default value if the column is null
    const std::string &getValueOfAuthors() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getAuthors() const noexcept;
    ///Set the value of the column authors
    void setAuthors(const std::string &pAuthors) noexcept;
    void setAuthors(std::string &&pAuthors) noexcept;

    /**  For column repository  */
    ///Get the value of the column repository, returns the default value if the column is null
    const std::string &getValueOfRepository() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getRepository() const noexcept;
    ///Set the value of the column repository
    void setRepository(const std::string &pRepository) noexcept;
    void setRepository(std::string &&pRepository) noexcept;

    /**  For column description  */
    ///Get the value of the column description, returns the default value if the column is null
    const std::string &getValueOfDescription() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getDescription() const noexcept;
    ///Set the value of the column description
    void setDescription(const std::string &pDescription) noexcept;
    void setDescription(std::string &&pDescription) noexcept;

    /**  For column license  */
    ///Get the value of the column license, returns the default value if the column is null
    const std::string &getValueOfLicense() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getLicense() const noexcept;
    ///Set the value of the column license
    void setLicense(const std::string &pLicense) noexcept;
    void setLicense(std::string &&pLicense) noexcept;

    /**  For column readme  */
    ///Get the value of the column readme, returns the default value if the column is null
    const std::string &getValueOfReadme() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getReadme() const noexcept;
    ///Set the value of the column readme
    void setReadme(const std::string &pReadme) noexcept;
    void setReadme(std::string &&pReadme) noexcept;
    void setReadmeToNull() noexcept;

    /**  For column sha256sum  */
    ///Get the value of the column sha256sum, returns the default value if the column is null
    const std::string &getValueOfSha256sum() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getSha256sum() const noexcept;
    ///Set the value of the column sha256sum
    void setSha256sum(const std::string &pSha256sum) noexcept;
    void setSha256sum(std::string &&pSha256sum) noexcept;


    static size_t getColumnNumber() noexcept {  return 12;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    /// Relationship interfaces
  private:
    friend drogon::orm::Mapper<Package>;
    friend drogon::orm::BaseBuilder<Package, true, true>;
    friend drogon::orm::BaseBuilder<Package, true, false>;
    friend drogon::orm::BaseBuilder<Package, false, true>;
    friend drogon::orm::BaseBuilder<Package, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<Package>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    ///For mysql or sqlite3
    void updateId(const uint64_t id);
    std::shared_ptr<std::string> id_;
    std::shared_ptr<::trantor::Date> publishedAt_;
    std::shared_ptr<std::string> metadata_;
    std::shared_ptr<std::string> name_;
    std::shared_ptr<std::string> version_;
    std::shared_ptr<short> edition_;
    std::shared_ptr<std::string> authors_;
    std::shared_ptr<std::string> repository_;
    std::shared_ptr<std::string> description_;
    std::shared_ptr<std::string> license_;
    std::shared_ptr<std::string> readme_;
    std::shared_ptr<std::string> sha256sum_;
    struct MetaData
    {
        const std::string colName_;
        const std::string colType_;
        const std::string colDatabaseType_;
        const ssize_t colLength_;
        const bool isAutoVal_;
        const bool isPrimaryKey_;
        const bool notNull_;
    };
    static const std::vector<MetaData> metaData_;
    bool dirtyFlag_[12]={ false };
  public:
    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="select * from " + tableName + " where id = $1";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where id = $1";
        return sql;
    }
    std::string sqlForInserting(bool &needSelection) const
    {
        std::string sql="insert into " + tableName + " (";
        size_t parametersCount = 0;
        needSelection = false;
        sql += "id,";
        ++parametersCount;
        if(!dirtyFlag_[0])
        {
            needSelection=true;
        }
        sql += "published_at,";
        ++parametersCount;
        if(!dirtyFlag_[1])
        {
            needSelection=true;
        }
        if(dirtyFlag_[2])
        {
            sql += "metadata,";
            ++parametersCount;
        }
        if(dirtyFlag_[3])
        {
            sql += "name,";
            ++parametersCount;
        }
        if(dirtyFlag_[4])
        {
            sql += "version,";
            ++parametersCount;
        }
        if(dirtyFlag_[5])
        {
            sql += "edition,";
            ++parametersCount;
        }
        if(dirtyFlag_[6])
        {
            sql += "authors,";
            ++parametersCount;
        }
        if(dirtyFlag_[7])
        {
            sql += "repository,";
            ++parametersCount;
        }
        if(dirtyFlag_[8])
        {
            sql += "description,";
            ++parametersCount;
        }
        if(dirtyFlag_[9])
        {
            sql += "license,";
            ++parametersCount;
        }
        if(dirtyFlag_[10])
        {
            sql += "readme,";
            ++parametersCount;
        }
        if(dirtyFlag_[11])
        {
            sql += "sha256sum,";
            ++parametersCount;
        }
        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        int placeholder=1;
        char placeholderStr[64];
        size_t n=0;
        if(dirtyFlag_[0])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[1])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[2])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[3])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[4])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[5])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[6])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[7])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[8])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[9])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[10])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[11])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(parametersCount > 0)
        {
            sql.resize(sql.length() - 1);
        }
        if(needSelection)
        {
            sql.append(") returning *");
        }
        else
        {
            sql.append(1, ')');
        }
        LOG_TRACE << sql;
        return sql;
    }
};
} // namespace postgres
} // namespace drogon_model
