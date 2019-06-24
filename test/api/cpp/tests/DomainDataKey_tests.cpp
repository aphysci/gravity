#include "DomainDataKey.h"
#include "doctest.h"

#include <string>
#include <iostream>

TEST_CASE("Tests for DomainDataKey") {
  
  GIVEN("two DomainDataKeys with empty domain and dataProductIDs") {
    DomainDataKey key1("",""); 
    DomainDataKey key2("",""); 
    THEN("they should be equal") {
      CHECK(key1 == key2);
      CHECK(key2 == key1);
      CHECK(!(key1 != key2));
      CHECK(!(key2 != key1));
    }
    THEN("they should not be less than or greater than") {
      CHECK(!(key1 < key2));
      CHECK(!(key2 < key1));
      CHECK(!(key1 > key2));
      CHECK(!(key2 > key1));
    }
  }

  GIVEN("the key2 domain > key1 domain but key2 dataProductID < key 1 dataProductID") {
    DomainDataKey key1("Domain1","DataProduct2"); 
    DomainDataKey key2("Domain2","DataProduct1"); 
    THEN("they should not be equal") {
      CHECK(key1 != key2);
      CHECK(key2 != key1);
      CHECK(!(key1 == key2));
      CHECK(!(key2 == key1));
    }
    THEN("key2 should be greater than key1") {
      CHECK((key1 < key2));
      CHECK((key2 > key1));
      CHECK(!(key1 > key2));
      CHECK(!(key2 < key1));
    }
  }

  GIVEN("equal domains, but key2 dataProduct < key1 data product") {
    DomainDataKey key1("Domain","DataProduct2"); 
    DomainDataKey key2("Domain","DataProduct1"); 
    THEN("they should not be equal") {
      CHECK(key1 != key2);
      CHECK(key2 != key1);
      CHECK(!(key1 == key2));
      CHECK(!(key2 == key1));
    }
    THEN("key2 should be less than key1") {
      CHECK(!(key1 < key2));
      CHECK(!(key2 > key1));
      CHECK((key1 > key2));
      CHECK((key2 < key1));
    }
  }

}

