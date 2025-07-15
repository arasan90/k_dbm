#include "k_dbm.h"

#include <gtest/gtest.h>
#include <k_dbm_priv.h>

#include "k_dbm_priv.h"

size_t insert_in_nvm_count	 = 0;
size_t get_from_nvm_count	 = 0;
size_t delete_from_nvm_count = 0;
size_t mutex_lock_count		 = 0;
size_t mutex_unlock_count	 = 0;

int test_mutex_lock(int timeout_ms)
{
	mutex_lock_count++;
	return 0;
}
void test_mutex_unlock() { mutex_unlock_count++; };
int	 test_dbm_insert(const char *key, const char *value)
{
	insert_in_nvm_count++;
	if (0 == strcmp(key, "key_fail"))
	{
		return -1;
	}
	return 0;
}
int test_dbm_get(const char *key, char *value, size_t value_buffer_size)
{
	get_from_nvm_count++;
	if (0 == strcmp(key, "non_existent_key"))
	{
		memset(value, 0, value_buffer_size);
		return -1;
	}
	else if (0 == strcmp(key, "nvmKey"))
	{
		strncpy(value, "nvmValue", value_buffer_size);
		return 0;
	}
	return 0;
}
int test_dbm_delete(const char *key)
{
	delete_from_nvm_count++;
	if (0 == strcmp(key, "key_delete_fail"))
	{
		return -1;
	}
	return 0;
}

extern k_dbm_context_t k_dbm_context;

class k_dbmTest : public ::testing::Test
{
   protected:
	void SetUp() override
	{
		k_dbm_init(&config);
		insert_in_nvm_count	  = 0;
		get_from_nvm_count	  = 0;
		delete_from_nvm_count = 0;
		mutex_lock_count	  = 0;
		mutex_unlock_count	  = 0;
		for (size_t i = 0; i < K_DBM_DB_SIZE; i++)
		{
			k_dbm_context.db.entries_a[i].storage  = K_DBM_STORAGE_NONE;
			k_dbm_context.db.entries_a[i].key	   = nullptr;
			k_dbm_context.db.entries_a[i].value[0] = '\0';
		}
	}

	const k_dbm_config_t config = {
		.k_dbm_lock_mutex_f	  = test_mutex_lock,
		.k_dbm_unlock_mutex_f = test_mutex_unlock,
		.k_dbm_insert_f		  = test_dbm_insert,
		.k_dbm_get_f		  = test_dbm_get,
		.k_dbm_delete_f		  = test_dbm_delete,
	};
};

TEST(k_dbm, initSuccess)
{
	const k_dbm_config_t config = {
		.k_dbm_lock_mutex_f	  = test_mutex_lock,
		.k_dbm_unlock_mutex_f = test_mutex_unlock,
		.k_dbm_insert_f		  = test_dbm_insert,
		.k_dbm_get_f		  = test_dbm_get,
		.k_dbm_delete_f		  = test_dbm_delete,
	};
	EXPECT_EQ(k_dbm_init(&config), 0);
}

TEST(k_dbm, initFailNullPointer) { EXPECT_EQ(k_dbm_init(nullptr), -1); }

TEST(k_dbm, initFailMissingFunction)
{
	k_dbm_config_t config = {
		.k_dbm_lock_mutex_f	  = nullptr,
		.k_dbm_unlock_mutex_f = test_mutex_unlock,
		.k_dbm_insert_f		  = test_dbm_insert,
		.k_dbm_get_f		  = test_dbm_get,
		.k_dbm_delete_f		  = test_dbm_delete,
	};
	EXPECT_EQ(k_dbm_init(&config), -1);

	config.k_dbm_lock_mutex_f	= test_mutex_lock;
	config.k_dbm_unlock_mutex_f = nullptr;
	EXPECT_EQ(k_dbm_init(&config), -1);

	config.k_dbm_unlock_mutex_f = test_mutex_unlock;
	config.k_dbm_insert_f		= nullptr;
	EXPECT_EQ(k_dbm_init(&config), -1);

	config.k_dbm_insert_f = test_dbm_insert;
	config.k_dbm_get_f	  = nullptr;
	EXPECT_EQ(k_dbm_init(&config), -1);

	config.k_dbm_get_f	  = test_dbm_get;
	config.k_dbm_delete_f = nullptr;
	EXPECT_EQ(k_dbm_init(&config), -1);
}

TEST_F(k_dbmTest, firstFreeEntryIs0) { EXPECT_EQ(k_dbm_find_first_empty_entry(), 0); }

TEST_F(k_dbmTest, firstFreeEntryIs2)
{
	k_dbm_context.db.entries_a[0].storage = K_DBM_STORAGE_RAM;
	k_dbm_context.db.entries_a[1].storage = K_DBM_STORAGE_NVM;
	EXPECT_EQ(k_dbm_find_first_empty_entry(), 2);
}

TEST_F(k_dbmTest, firstFreeEntryIsLastOne)
{
	for (size_t i = 0; i < K_DBM_DB_SIZE - 1; i++)
	{
		k_dbm_context.db.entries_a[i].storage = K_DBM_STORAGE_RAM;
	}
	EXPECT_EQ(k_dbm_find_first_empty_entry(), K_DBM_DB_SIZE - 1);
}

TEST_F(k_dbmTest, noFreeEntries)
{
	for (size_t i = 0; i < K_DBM_DB_SIZE; i++)
	{
		k_dbm_context.db.entries_a[i].storage = K_DBM_STORAGE_RAM;
	}
	EXPECT_EQ(k_dbm_find_first_empty_entry(), -1);
}

TEST_F(k_dbmTest, findEntryNotFound) { EXPECT_EQ(k_dbm_find_entry("non_existent_key"), -1); }

TEST_F(k_dbmTest, findEntryFound)
{
	k_dbm_context.db.entries_a[0].key	  = "key";
	k_dbm_context.db.entries_a[0].storage = K_DBM_STORAGE_RAM;
	EXPECT_EQ(k_dbm_find_entry("key"), 0);
}

TEST_F(k_dbmTest, findEntryFoundInNVM)
{
	k_dbm_context.db.entries_a[29].key	   = "nvmKey";
	k_dbm_context.db.entries_a[29].storage = K_DBM_STORAGE_NVM;
	EXPECT_EQ(k_dbm_find_entry("nvmKey"), 29);
}

TEST_F(k_dbmTest, insertInRamSuccess)
{
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_RAM), 0);
	EXPECT_STREQ(k_dbm_context.db.entries_a[0].key, "key");
	EXPECT_STREQ(k_dbm_context.db.entries_a[0].value, "value");
	EXPECT_EQ(k_dbm_context.db.entries_a[0].storage, K_DBM_STORAGE_RAM);
	EXPECT_EQ(insert_in_nvm_count, 0);
}

TEST_F(k_dbmTest, insertInNVMSuccess)
{
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_NVM), 0);
	EXPECT_STREQ(k_dbm_context.db.entries_a[0].key, "key");
	EXPECT_STREQ(k_dbm_context.db.entries_a[0].value, "value");
	EXPECT_EQ(k_dbm_context.db.entries_a[0].storage, K_DBM_STORAGE_NVM);
	EXPECT_EQ(insert_in_nvm_count, 1);
}

TEST_F(k_dbmTest, insertInNVMFail)
{
	EXPECT_EQ(k_dbm_insert("key_fail", "value", K_DBM_STORAGE_NVM), -1);
	EXPECT_EQ(k_dbm_context.db.entries_a[0].key, nullptr);
	EXPECT_STREQ(k_dbm_context.db.entries_a[0].value, "");
	EXPECT_EQ(k_dbm_context.db.entries_a[0].storage, K_DBM_STORAGE_NONE);
	EXPECT_EQ(insert_in_nvm_count, 1);
}

TEST_F(k_dbmTest, updateKeyInRAM)
{
	char value_buffer[32] = {0};
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_RAM), 0);
	EXPECT_EQ(mutex_lock_count, 1);
	EXPECT_EQ(mutex_unlock_count, 1);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(k_dbm_get("key", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_STREQ(value_buffer, "value");
	EXPECT_EQ(k_dbm_insert("key", "new_value", K_DBM_STORAGE_RAM), 0);
	EXPECT_EQ(mutex_lock_count, 3);
	EXPECT_EQ(mutex_unlock_count, 3);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(k_dbm_get("key", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_EQ(mutex_lock_count, 4);
	EXPECT_EQ(mutex_unlock_count, 4);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_STREQ(value_buffer, "new_value");
}

TEST_F(k_dbmTest, updateKeyInNVM)
{
	char value_buffer[32] = {0};
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_NVM), 0);
	EXPECT_EQ(mutex_lock_count, 1);
	EXPECT_EQ(mutex_unlock_count, 1);
	EXPECT_EQ(insert_in_nvm_count, 1);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(k_dbm_get("key", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
	EXPECT_EQ(insert_in_nvm_count, 1);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_STREQ(value_buffer, "value");
	EXPECT_EQ(k_dbm_insert("key", "new_value", K_DBM_STORAGE_NVM), 0);
	EXPECT_EQ(mutex_lock_count, 3);
	EXPECT_EQ(mutex_unlock_count, 3);
	EXPECT_EQ(insert_in_nvm_count, 2);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(k_dbm_get("key", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_EQ(mutex_lock_count, 4);
	EXPECT_EQ(mutex_unlock_count, 4);
	EXPECT_EQ(insert_in_nvm_count, 2);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_STREQ(value_buffer, "new_value");
}

TEST_F(k_dbmTest, getNotExistentKey)
{
	char value_buffer[32] = {0};
	EXPECT_EQ(k_dbm_get("non_existent_key", value_buffer, sizeof(value_buffer)), -1);
	EXPECT_STREQ(value_buffer, "");
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(mutex_lock_count, 1);
	EXPECT_EQ(mutex_unlock_count, 1);
	EXPECT_EQ(get_from_nvm_count, 1);
}

TEST_F(k_dbmTest, getExistentKeyFromRAM)
{
	char value_buffer[32] = {0};
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_RAM), 0);
	EXPECT_EQ(k_dbm_get("key", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_STREQ(value_buffer, "value");
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
}

TEST_F(k_dbmTest, getExistentKeyFromNVM)
{
	char value_buffer[32] = {0};
	EXPECT_EQ(k_dbm_get("nvmKey", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_STREQ(value_buffer, "nvmValue");
	EXPECT_EQ(get_from_nvm_count, 1);
	EXPECT_EQ(mutex_lock_count, 1);
	EXPECT_EQ(mutex_unlock_count, 1);
	memset(value_buffer, 0, sizeof(value_buffer));
	EXPECT_EQ(k_dbm_get("nvmKey", value_buffer, sizeof(value_buffer)), 0);
	EXPECT_STREQ(value_buffer, "nvmValue");
	EXPECT_EQ(get_from_nvm_count, 1);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
}

TEST_F(k_dbmTest, deleteNotExistingKey)
{
	EXPECT_EQ(k_dbm_delete("non_existent_key"), -1);
	EXPECT_EQ(mutex_lock_count, 1);
	EXPECT_EQ(mutex_unlock_count, 1);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(delete_from_nvm_count, 0);
}

TEST_F(k_dbmTest, deleteExistingKeyInRAM)
{
	EXPECT_EQ(k_dbm_insert("key", "value", K_DBM_STORAGE_RAM), 0);
	EXPECT_EQ(k_dbm_delete("key"), 0);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
	EXPECT_EQ(insert_in_nvm_count, 0);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(delete_from_nvm_count, 0);
	EXPECT_EQ(k_dbm_find_entry("key"), -1);
}

TEST_F(k_dbmTest, deleteExistingKeyInNVM)
{
	EXPECT_EQ(k_dbm_insert("nvmKey", "nvmValue", K_DBM_STORAGE_NVM), 0);
	EXPECT_EQ(k_dbm_delete("nvmKey"), 0);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
	EXPECT_EQ(insert_in_nvm_count, 1);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(delete_from_nvm_count, 1);
	EXPECT_EQ(k_dbm_find_entry("nvmKey"), -1);
}

TEST_F(k_dbmTest, deleteExistingKeyInNVMFail)
{
	EXPECT_EQ(k_dbm_insert("key_delete_fail", "value", K_DBM_STORAGE_NVM), 0);
	EXPECT_EQ(k_dbm_delete("key_delete_fail"), -1);
	EXPECT_EQ(mutex_lock_count, 2);
	EXPECT_EQ(mutex_unlock_count, 2);
	EXPECT_EQ(insert_in_nvm_count, 1);
	EXPECT_EQ(get_from_nvm_count, 0);
	EXPECT_EQ(delete_from_nvm_count, 1);
	EXPECT_NE(k_dbm_find_entry("key_delete_fail"), -1);
}

TEST_F(k_dbmTest, getDbEmpty)
{
	size_t free_space = k_dbm_get_free_space();
	EXPECT_EQ(free_space, K_DBM_DB_SIZE);
}

TEST_F(k_dbmTest, getDbWithEntries)
{
	EXPECT_EQ(k_dbm_insert("key1", "value1", K_DBM_STORAGE_RAM), 0);
	EXPECT_EQ(k_dbm_insert("key2", "value2", K_DBM_STORAGE_NVM), 0);
	EXPECT_EQ(k_dbm_insert("key3", "value3", K_DBM_STORAGE_RAM), 0);
	size_t free_space = k_dbm_get_free_space();
	EXPECT_EQ(free_space, K_DBM_DB_SIZE - 3);
	EXPECT_NE(k_dbm_find_entry("key1"), -1);
	EXPECT_NE(k_dbm_find_entry("key2"), -1);
	EXPECT_NE(k_dbm_find_entry("key3"), -1);
}