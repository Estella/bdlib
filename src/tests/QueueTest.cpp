/* QueueTest.c
 *
 */
#include "QueueTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION (QueueTest);

void QueueTest :: setUp (void)
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
  a = new Queue<String>();
  b = new Queue<String>();
  c = new Queue<String>();
  d = new Queue<String>();
  e = new Queue<String*>();
  f = new Queue<String*>();
  g = new Queue<String*>();
  h = new Queue<String*>();
}

void QueueTest :: tearDown (void)
{
    // finally delete objects
    delete a; delete b; delete c; delete d;
    delete e; delete f; delete g; delete h;
}

void QueueTest :: enqueueTest (void)
{
  CPPUNIT_ASSERT_EQUAL((size_t)0, a->size());
  a->enqueue("Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());
  a->enqueue("Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)2, a->size());
  a->enqueue("Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)3, a->size());
  a->enqueue("Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());
  a->enqueue("Blah");
  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());

  (*a) << "Blah1" << "Blah2";
  CPPUNIT_ASSERT_EQUAL((size_t)7, a->size());
}

void QueueTest :: dequeueTest (void)
{
  a->enqueue("Blahstart");
  a->enqueue("Bleck");
  a->enqueue("Blah");
  a->enqueue("Blah");
  a->enqueue("Blahend");

  CPPUNIT_ASSERT_EQUAL((size_t)5, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("Blahstart", a->dequeue());

  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());

  CPPUNIT_ASSERT_STRING_EQUAL("Bleck", *(a->front()));
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());

  CPPUNIT_ASSERT_STRING_EQUAL("Bleck", *(a->front()));
  CPPUNIT_ASSERT_EQUAL((size_t)4, a->size());

  CPPUNIT_ASSERT_STRING_EQUAL("Bleck", a->dequeue());
  
  CPPUNIT_ASSERT_EQUAL((size_t)3, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", a->dequeue());
  
  CPPUNIT_ASSERT_EQUAL((size_t)2, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("Blah", a->dequeue());
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("Blahend", a->dequeue());
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, a->size());

  (*a) << "One" << "Two" << "Three";
  String one, two, three;
  (*a) >> one >> two;
  CPPUNIT_ASSERT_EQUAL((size_t)1, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("One", one);
  CPPUNIT_ASSERT_STRING_EQUAL("Two", two);
  (*a) >> three;
  CPPUNIT_ASSERT_EQUAL((size_t)0, a->size());
  CPPUNIT_ASSERT_STRING_EQUAL("Three", three);
}

void QueueTest :: iterateTest (void)
{
  (*a) << "1" << "2" << "3" << "4";
  Queue<String>::iterator iter(a->begin());
  int count = 1;
  for (iter = a->begin(); iter != a->end(); ++iter) {
    char buf[2] = "";
    sprintf(buf, "%d", count);
    ++count;
    CPPUNIT_ASSERT_STRING_EQUAL(buf, *iter);
  }
}
