#pragma once

#include <Formats/FormatSettings.h>
#include <Storages/IStorage.h>
#include <Storages/MergeTree/MergeTreeData.h>


namespace DB
{

class Context;

struct StoragesInfo
{
    StoragePtr storage = nullptr;
    TableLockHolder table_lock;

    String database;
    String table;
    String engine;

    bool need_inactive_parts = false;
    MergeTreeData * data = nullptr;

    explicit operator bool() const { return storage != nullptr; }
    MergeTreeData::DataPartsVector
    getParts(MergeTreeData::DataPartStateVector & state, bool has_state_column, bool require_projection_parts = false) const;
};

/** A helper class that enumerates the storages that match given query. */
class StoragesInfoStream
{
public:
    StoragesInfoStream(const SelectQueryInfo & query_info, ContextPtr context);
    StoragesInfo next();

private:
    String query_id;
    Settings settings;


    ColumnPtr database_column;
    ColumnPtr table_column;
    ColumnPtr active_column;

    size_t next_row;
    size_t rows;

    using StoragesMap = std::map<std::pair<String, String>, StoragePtr>;
    StoragesMap storages;
};

/** Implements system table 'parts' which allows to get information about data parts for tables of MergeTree family.
  */
class StorageSystemPartsBase : public IStorage
{
public:
    Pipe read(
        const Names & column_names,
        const StorageSnapshotPtr & storage_snapshot,
        SelectQueryInfo & query_info,
        ContextPtr context,
        QueryProcessingStage::Enum processed_stage,
        size_t max_block_size,
        unsigned num_streams) override;

    NamesAndTypesList getVirtuals() const override;

    bool isSystemStorage() const override { return true; }

private:
    static bool hasStateColumn(const Names & column_names, const StorageSnapshotPtr & storage_snapshot);

protected:
    const FormatSettings format_settings;

    StorageSystemPartsBase(const StorageID & table_id_, NamesAndTypesList && columns_);

    virtual void
    processNextStorage(ContextPtr context, MutableColumns & columns, std::vector<UInt8> & columns_mask, const StoragesInfo & info, bool has_state_column) = 0;
};

}
