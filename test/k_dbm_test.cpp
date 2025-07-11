#include "k_dbm.h"

#include <gtest/gtest.h>
#include <k_dbm_priv.h>

#include "k_dbm_priv.h"

size_t insert_in_nvm_count = 0;

int	 test_mutex_lock(int timeout_ms) { return 0; }
void test_mutex_unlock() {};
int	 test_dbm_insert(const char *key, const char *value)
{
	insert_in_nvm_count++;
	if (0 == strcmp(key, "key_fail"))
	{
		return -1;
	}
	return 0;
}
int test_dbm_get(const char *key, char **value) { return 0; }
int test_dbm_delete(const char *key) { return 0; }

extern k_dbm_context_t k_dbm_context;

class k_dbmTest : public ::testing::Test
{
   protected:
	void SetUp() override
	{
		k_dbm_init(&config);
		insert_in_nvm_count = 0;
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