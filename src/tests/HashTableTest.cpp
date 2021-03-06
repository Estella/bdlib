/* HashTableTest.c
 *
 */
#include "HashTableTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION (HashTableTest);

void HashTableTest :: setUp (void)
{
/*
    // set up test environment (initializing objects)
    a = new String();
    b = new String("blah");
    c = new String(*b);
    d = new String(cstring);
    e = new String(*d);
    f = new String(cstring, 11);
    g = new String('x');
    h = new String(35);
*/
  a = new HashTable<int, String>();
  b = new HashTable<int, String>();
  sa = new HashTable<String, String>();
  sb = new HashTable<String, String>();
}

void HashTableTest :: tearDown (void)
{
    // finally delete objects
    delete a;
    delete b;
    delete sa;
    delete sb;
/*
    delete b; delete c; delete d;
    delete e; delete f; delete g; delete h;
*/
}

void HashTableTest :: insertTest (void)
{
  CPPUNIT_ASSERT_EQUAL((size_t)0, a->size());
  a->insert(1, "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());
  a->insert(1, "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());
  a->insert(2, "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)2, a->size());
  a->insert(8, "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)3, a->size());
  a->insert(4, "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());

  b->insert(27, "Test");
  CPPUNIT_ASSERT_EQUAL((size_t)1, b->size());

  (*b) = (*a);
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());
  CPPUNIT_ASSERT_EQUAL((size_t)4, b->size());

  (*a)[42] = "42";
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());


  CPPUNIT_ASSERT_EQUAL((size_t)0, sa->size());
  sa->insert("1", "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)1, sa->size());
  sa->insert("1", "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)1, sa->size());
  sa->insert("2", "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)2, sa->size());
  sa->insert("8", "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)3, sa->size());
  sa->insert("4", "Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)4, sa->size());

  sb->insert("27", "Test");
  CPPUNIT_ASSERT_EQUAL((size_t)1, sb->size());

  (*sb) = (*sa);
  CPPUNIT_ASSERT_EQUAL((size_t)4, sa->size());
  CPPUNIT_ASSERT_EQUAL((size_t)4, sb->size());

  (*sa)["hmmzz"] = "hmmm";
  CPPUNIT_ASSERT_EQUAL((size_t)5, sa->size());

  (*sa)["7"] = "seven";
  CPPUNIT_ASSERT_EQUAL((size_t)6, sa->size());
}

void HashTableTest :: containsTest (void)
{
  a->insert(1, "Blah");
  a->insert(1, "Bleck");
  a->insert(2, "Blah");
  a->insert(8, "Blah");
  a->insert(4, "Blah");
  
  CPPUNIT_ASSERT_EQUAL(true, a->contains(1));
  CPPUNIT_ASSERT_EQUAL(false, a->contains(3));
  CPPUNIT_ASSERT_EQUAL(true, a->contains(4));

  (*b) = (*a);

  CPPUNIT_ASSERT_EQUAL(true, b->contains(1));
  CPPUNIT_ASSERT_EQUAL(false, b->contains(3));
  CPPUNIT_ASSERT_EQUAL(true, b->contains(4));

  b->insert(5, "test");
  CPPUNIT_ASSERT_EQUAL(true, b->contains(5));
  CPPUNIT_ASSERT_EQUAL(false, a->contains(5));

  (*b) = (*a);
  CPPUNIT_ASSERT_EQUAL(true, b->contains(1));
  CPPUNIT_ASSERT_EQUAL(false, b->contains(3));
  CPPUNIT_ASSERT_EQUAL(true, b->contains(4));
  CPPUNIT_ASSERT_EQUAL(false, b->contains(5));
  CPPUNIT_ASSERT_EQUAL(false, a->contains(5));

  (*a)[42] = "42";
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());
  CPPUNIT_ASSERT_EQUAL(true, a->contains(42));


  sa->insert("1", "Blah");
  sa->insert("1", "Bleck");
  sa->insert("2", "Blah");
  sa->insert("8", "Blah");
  sa->insert("4", "Blah");
  
  CPPUNIT_ASSERT_EQUAL(true, sa->contains("1"));
  CPPUNIT_ASSERT_EQUAL(false, sa->contains("3"));
  CPPUNIT_ASSERT_EQUAL(true, sa->contains("4"));

  (*sb) = (*sa);

  CPPUNIT_ASSERT_EQUAL(true, sb->contains("1"));
  CPPUNIT_ASSERT_EQUAL(false, sb->contains("3"));
  CPPUNIT_ASSERT_EQUAL(true, sb->contains("4"));

  sb->insert("5", "test");
  CPPUNIT_ASSERT_EQUAL(true, sb->contains("5"));
  CPPUNIT_ASSERT_EQUAL(false, sa->contains("5"));

  (*sb) = (*sa);
  CPPUNIT_ASSERT_EQUAL(true, sb->contains("1"));
  CPPUNIT_ASSERT_EQUAL(false, sb->contains("3"));
  CPPUNIT_ASSERT_EQUAL(true, sb->contains("4"));
  CPPUNIT_ASSERT_EQUAL(false, sb->contains("5"));
  CPPUNIT_ASSERT_EQUAL(false, sa->contains("5"));

  (*sa)["42"] = "42";
  CPPUNIT_ASSERT_EQUAL((size_t)5, sa->size());
  CPPUNIT_ASSERT_EQUAL(true, sa->contains("42"));


  //Test .keys and .values
  Array<String> keys = sa->keys();
  CPPUNIT_ASSERT_EQUAL(bool(1), keys.find("1") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(bool(1), keys.find("2") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(bool(1), keys.find("8") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(bool(1), keys.find("4") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(bool(1), keys.find("42") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(size_t(5), keys.size());

  Array<String> values = sa->values();
  CPPUNIT_ASSERT_EQUAL(bool(1), values.find("Blah") != size_t(-1));
  CPPUNIT_ASSERT_EQUAL(bool(1), values.find("42") != size_t(-1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah Blah Blah Blah 42", values.join(' '));
  CPPUNIT_ASSERT_EQUAL(size_t(5), values.size());
}

void HashTableTest :: clearTest (void)
{
    a->insert(1, "Blah");
    a->insert(1, "Bleck");
    a->insert(2, "Blah");
    a->insert(8, "Blah");
    a->insert(4, "Blah");

    CPPUNIT_ASSERT_EQUAL((size_t)4, (*a).size());

    a->clear();

    CPPUNIT_ASSERT_EQUAL((size_t)0, (*a).size());
    CPPUNIT_ASSERT_EQUAL(true, a->isEmpty());

    a->insert(1, "Blah1");
    a->insert(2, "Blah2");
    a->insert(3, "Blah3");

    CPPUNIT_ASSERT_EQUAL((size_t)3, (*a).size());
    CPPUNIT_ASSERT_EQUAL(true, a->contains(1));
    CPPUNIT_ASSERT_EQUAL(true, a->contains(2));
    CPPUNIT_ASSERT_EQUAL(true, a->contains(3));


    sa->insert("1", "Blah");
    sa->insert("1", "Bleck");
    sa->insert("2", "Blah");
    sa->insert("8", "Blah");
    sa->insert("4", "Blah");

    CPPUNIT_ASSERT_EQUAL((size_t)4, (*sa).size());

    sa->clear();

    CPPUNIT_ASSERT_EQUAL((size_t)0, (*sa).size());
    CPPUNIT_ASSERT_EQUAL(true, sa->isEmpty());

    sa->insert("1", "Blah1");
    sa->insert("2", "Blah2");
    sa->insert("3", "Blah3");

    CPPUNIT_ASSERT_EQUAL((size_t)3, (*sa).size());
    CPPUNIT_ASSERT_EQUAL(true, sa->contains("1"));
    CPPUNIT_ASSERT_EQUAL(true, sa->contains("2"));
    CPPUNIT_ASSERT_EQUAL(true, sa->contains("3"));
}

void HashTableTest :: getValueTest (void)
{
  a->insert(1, "Blah1");
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", a->getValue(1));
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());

  CPPUNIT_ASSERT_EQUAL(false, a->insert(1, "Bleck"));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", a->getValue(1));
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());


  a->insert(2, "Blah2");
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", a->getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", a->getValue(2));
  CPPUNIT_ASSERT_EQUAL((size_t)2, a->size());

  a->insert(8, "Blah8");
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", a->getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", a->getValue(2));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", a->getValue(8));
  CPPUNIT_ASSERT_EQUAL((size_t)3, a->size());

  a->insert(4, "Blah4");
  CPPUNIT_ASSERT_STRING_EQUAL("Blah4", (*a)[4]);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", (*a)[1]);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", (*a)[2]);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", a->getValue(8));
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());

  
  String result = (*a)[58];
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());

  (*b) = (*a);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah4", b->getValue(4));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", b->getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", b->getValue(2));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", b->getValue(8));
  CPPUNIT_ASSERT_EQUAL((size_t)5, b->size());
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());

  HashTable<int, String> test(*a);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah4", test.getValue(4));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah1", test.getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", test.getValue(2));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", test.getValue(8));
  CPPUNIT_ASSERT_EQUAL((size_t)5, test.size());
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());

  (*a)[42] = "42";
  CPPUNIT_ASSERT_STRING_EQUAL("42", (*a)[42]);

}

#ifdef DISABLED
void HashTableTest :: getKeyTest (void)
{
  a->insert(1, "Blah");
  a->insert(1, "Bleck");
  a->insert(2, "Blah2");
  a->insert(8, "Blah8");
  a->insert(4, "Blah4");
  
  CPPUNIT_ASSERT_EQUAL(1, a->getKey("Blah"));
  CPPUNIT_ASSERT_EQUAL(2, a->getKey("Blah2"));
  CPPUNIT_ASSERT_EQUAL(8, a->getKey("Blah8"));
  CPPUNIT_ASSERT_EQUAL(4, a->getKey("Blah4"));
}
#endif

void HashTableTest :: removeTest (void)
{
  a->insert(1, "Blah");
  a->insert(1, "Bleck");
  a->insert(2, "Blah2");
  a->insert(8, "Blah8");
  a->insert(4, "Blah4");

  a->remove(8);  

  String result;

  result = a->getValue(58);
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());

 
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", a->getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah4", a->getValue(4));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", a->getValue(2));
  result = a->getValue(8);
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());

  CPPUNIT_ASSERT_EQUAL(false, a->remove(8));

  a->remove(4);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", a->getValue(1));
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", a->getValue(2));
  result = a->getValue(4);
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());

  a->remove(2);
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", a->getValue(1));
  result = a->getValue(2);
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());

  a->remove(1);
  CPPUNIT_ASSERT_EQUAL(true, a->isEmpty());
  result = a->getValue(1);
  CPPUNIT_ASSERT_EQUAL(true, result.isEmpty());
}

#ifdef disabled
void HashTableTest :: iterateTest (void)
{
  a->insert(1, "Blah");
  a->insert(1, "Bleck");
  a->insert(2, "Blah2");
  a->insert(8, "Blah8");
  a->insert(4, "Blah4");

  HashTable<int, String>::iterator iter = a->begin();
  while (iter.hasNext()) {
//    int key = (int) iter.next();
//    printf("%d\n", key);
    String value = iter.next();
//    printf("%d: %s\n", key, a->getValue(key));
  }

  iter = a->begin();
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah4", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", iter.next());

  a->remove(4);
  iter = a->begin();
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah2", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", iter.next());

  a->remove(2);
  iter = a->begin();
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", iter.next());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah8", iter.next());

  a->remove(8);
  iter = a->begin();
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", iter.next());

  a->remove(1);
  iter = a->begin();
  CPPUNIT_ASSERT_EQUAL(false, iter.hasNext());
}
#endif
