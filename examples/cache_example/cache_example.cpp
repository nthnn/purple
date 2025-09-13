#include <iostream>
#include <purple/memcache/cache.hpp>

using namespace Purple::MemoryCache;

void print_cache_stats(CacheStats stats) {
  std::cout << "--- Cache Statistics ---" << std::endl;

  std::cout << "Hits: " << stats.hits << std::endl;

  std::cout << "Misses: " << stats.misses << std::endl;

  std::cout << "Current Size (Bytes): " << stats.current_size_bytes
            << std::endl;

  std::cout << "Current Item Count: " << stats.current_item_count << std::endl;

  std::cout << "Total Evictions: " << stats.evictions << std::endl;

  std::cout << "Expired Evictions: " << stats.expired_evictions << std::endl;

  std::cout << "Capacity Evictions: " << stats.capacity_evictions << std::endl;

  std::cout << "Priority Evictions: " << stats.priority_evictions << std::endl;

  std::cout << "------------------------" << std::endl;
}

int main() {
  std::shared_ptr<ICache<std::string, int>> int_cache =
      CacheManager<std::string, int>::get_cache("myint_cache", 1024 * 100, 50);

  std::cout << "--- Testing Int Cache ---" << std::endl;

  int_cache->put("item1", 100, 5, 0, 10);
  int_cache->put("item2", 200, 0, 0, 5);
  int_cache->put("item3", 300, 0, 0, 0);
  int_cache->put("item4", 400, 0, 0, 20);

  int val;
  if (int_cache->get("item1", val))
    std::cout << "Retrieved item1: " << val << std::endl;
  else
    std::cout << "item1 not found or expired." << std::endl;

  std::cout << "Contains item2: "
            << (int_cache->contains("item2") ? "Yes" : "No") << std::endl;
  print_cache_stats(int_cache->get_stats());

  std::cout << "Waiting 6 seconds for item1 to expire..." << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(6));

  if (int_cache->get("item1", val))
    std::cout << "Retrieved item1: " << val << std::endl;
  else
    std::cout << "item1 not found or expired as expected." << std::endl;

  std::cout << "\nPutting many items to test eviction with priorities..."
            << std::endl;

  for (int i = 0; i < 60; ++i) {
    int_cache->put("priority_item_" + std::to_string(i), i, 3600, 100, i / 10);
  }

  std::cout << "Current item count after evictions: "
            << int_cache->get_stats().current_item_count << std::endl;
  print_cache_stats(int_cache->get_stats());

  std::cout << "\nChecking if low priority items were evicted..." << std::endl;

  if (int_cache->contains("priority_item_0"))
    std::cout
        << "priority_item_0 FOUND (Priority 0) - "
           "this might indicate that higher priority items were added later."
        << std::endl;
  else
    std::cout << "priority_item_0 NOT FOUND (Priority 0) - "
                 "as expected, lower priority item evicted."
              << std::endl;

  if (int_cache->contains("priority_item_59"))
    std::cout << "priority_item_59 FOUND (Priority 5) - "
                 "as expected, higher priority item retained."
              << std::endl;
  else
    std::cout << "priority_item_59 NOT FOUND (Priority 5) - "
                 "unexpected."
              << std::endl;

  int_cache->remove("item2");
  std::cout << "After removing item2, contains item2: "
            << (int_cache->contains("item2") ? "Yes" : "No") << std::endl;
  print_cache_stats(int_cache->get_stats());

  std::shared_ptr<ICache<std::string, std::string>> str_cache =
      CacheManager<std::string, std::string>::get_cache("mystr_cache");

  std::cout << "\n--- Testing String Cache ---" << std::endl;

  str_cache->put("greeting", "Hello, Cache!", 10, 0, 100);
  str_cache->put("message", "This is a test message.", 0, 0, 50);

  std::string str_val;
  if (str_cache->get("greeting", str_val))
    std::cout << "Retrieved greeting: " << str_val << std::endl;
  print_cache_stats(str_cache->get_stats());

  std::shared_ptr<ICache<std::string, std::vector<int>>> vec_cache =
      CacheManager<std::string, std::vector<int>>::get_cache(
          "vector_value_cache", 1024 * 500, 10);

  std::cout << "\n--- Testing Vector Cache ---" << std::endl;

  std::vector<int> vector_value = {1, 2, 3, 4, 5};
  vec_cache->put("vec1", vector_value, 0, 0, 8);

  std::vector<int> retrieved_vec;
  if (vec_cache->get("vec1", retrieved_vec)) {
    std::cout << "Retrieved vec1: ";

    for (int x : retrieved_vec)
      std::cout << x << " ";
    std::cout << std::endl;
  }
  print_cache_stats(vec_cache->get_stats());

  std::shared_ptr<ICache<std::string, const char *>> cstr_cache =
      CacheManager<std::string, const char *>::get_cache("myCstr_cache");
  cstr_cache->put("c_str1", "This is a C-style string", 0, 0, 1);

  const char *retrieved_c_str;
  if (cstr_cache->get("c_str1", retrieved_c_str))
    std::cout << "Retrieved c_str1: " << retrieved_c_str << std::endl;
  print_cache_stats(cstr_cache->get_stats());

  int_cache->clear();
  std::cout << "\n--- After clearing myint_cache ---" << std::endl;
  print_cache_stats(int_cache->get_stats());

  CacheManager<std::string, int>::remove_cache("myint_cache");

  CacheManager<std::string, std::string>::remove_cache("mystr_cache");

  CacheManager<std::string, std::vector<int>>::remove_cache(
      "vector_value_cache");

  CacheManager<std::string, const char *>::remove_cache("myCstr_cache");

  std::cout << "\nProgram finished." << std::endl;

  return 0;
}
