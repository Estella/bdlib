/* StreamTest.c
 *
 */
#include "StreamTest.h"
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION (StreamTest);

void StreamTest :: setUp (void)
{
    strcpy(cstring, "Some static cstring to play with");
    // set up test environment (initializing objects)
    a = new Stream();
    b = new Stream("blah");
    c = new Stream(*b);
    d = new Stream(cstring);
    e = new Stream(*d);
    F = new Stream(String(cstring, 11));
    g = new Stream('x');
    h = new Stream(35);
}

void StreamTest :: tearDown (void)
{
    // finally delete objects
    delete a; delete b; delete c; delete d;
    delete e; delete F; delete g; delete h;
}

void StreamTest :: reserveTest (void)
{
  a->Reserve(2000);
  CPPUNIT_ASSERT((a->capacity() % 2048) == 0);

  b->Reserve(10);
  CPPUNIT_ASSERT(b->capacity() >= 1024);
  CPPUNIT_ASSERT(h->capacity() >= 1024);
  CPPUNIT_ASSERT((b->capacity() % 1024) == 0);

  char buf[2050];
  memset(buf, 'c', sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = 0;
  g->puts(buf);

  CPPUNIT_ASSERT((g->capacity() >= 2048));

  /* Reference counted */
  b->puts(buf);
  CPPUNIT_ASSERT((b->capacity() >= 2048));
}

void StreamTest :: tellTest (void)
{
  CPPUNIT_ASSERT_EQUAL((size_t) 0, a->tell());
  CPPUNIT_ASSERT_EQUAL((size_t) 0, b->tell());
  CPPUNIT_ASSERT_EQUAL((size_t) 0, h->tell());
}

void StreamTest :: seekTest (void)
{
  a->seek(0, SEEK_END);
  CPPUNIT_ASSERT_EQUAL((size_t) 0, a->tell());

  b->seek(0, SEEK_END);
  CPPUNIT_ASSERT_EQUAL(b->capacity(), b->tell());

  b->seek(0, SEEK_SET);
  CPPUNIT_ASSERT_EQUAL((size_t) 0, b->tell());

  b->seek(1, SEEK_SET);
  CPPUNIT_ASSERT_EQUAL((size_t) 1, b->tell());

  b->seek(1, SEEK_END);
  CPPUNIT_ASSERT_EQUAL(b->capacity() - 1, b->tell());

}

void StreamTest :: putsTest (void)
{
  a->puts("This is a test of my buffer\n");
  a->puts("This is line 2, isn't it amazing?\n");
  CPPUNIT_ASSERT(*a == "This is a test of my buffer\nThis is line 2, isn't it amazing?\n");
  CPPUNIT_ASSERT_EQUAL((size_t) 62, a->length());
  *b = *a;
  a->seek(10, SEEK_SET);
  a->puts("look");
  CPPUNIT_ASSERT(*a == "This is a look of my buffer\nThis is line 2, isn't it amazing?\n");
  CPPUNIT_ASSERT_EQUAL((size_t) 62, a->length());
  CPPUNIT_ASSERT_EQUAL(b->capacity(), a->capacity());
  CPPUNIT_ASSERT_EQUAL(b->length(), a->length());

  a->clear();
  b->clear();

  *a << "This is a test of my buffer\n";
  *a << "This is line 2, isn't it amazing?\n";
  CPPUNIT_ASSERT(*a == "This is a test of my buffer\nThis is line 2, isn't it amazing?\n");
  CPPUNIT_ASSERT_EQUAL((size_t) 62, a->length());
  *b = *a;
  a->seek(10, SEEK_SET);
  *a << "look";
  CPPUNIT_ASSERT(*a == "This is a look of my buffer\nThis is line 2, isn't it amazing?\n");
  CPPUNIT_ASSERT_EQUAL((size_t) 62, a->length());
  CPPUNIT_ASSERT_EQUAL(b->capacity(), a->capacity());
  CPPUNIT_ASSERT_EQUAL(b->length(), a->length());
}

void StreamTest :: truncateTest (void)
{
  a->puts("This is a test of my buffer\n");
  a->puts("This is line 2, isn't it amazing?\n");
  a->clear();
  CPPUNIT_ASSERT_EQUAL((size_t) 0, a->length());
  CPPUNIT_ASSERT(*a == "");

  a->puts("This is a test of my buffer\n");
  a->puts("This is line 2, isn't it amazing?\n");
  a->seek(10, SEEK_SET);
  a->truncate();
  CPPUNIT_ASSERT_EQUAL((size_t) 10, a->length());
  CPPUNIT_ASSERT(*a == "This is a ");
}

void StreamTest :: readTest (void)
{
  String sbuf, gbuf;

  a->puts("This is line 1\n");
  a->puts("This is line 2\n");
  a->puts("This is line 3\n");
  a->puts("This is line 4\n");
  a->puts("This is line 5\n");
  a->puts("This is line 6\n");
  a->puts("This is line 7\n");
  a->puts("This is line 8\n");
  a->puts("This is line 9\n");
  a->puts("This is line10\n");
  a->puts("This is line11\n");
  a->puts("This is line12\n");
  a->puts("This is line13\n");
  a->puts("This is line14\n");
  a->seek(0, SEEK_SET);
  int x = 0;
  size_t pos = 0;
  while (a->tell() < a->length()) {
    sbuf = String::printf("This is line%2d\nThis is line%2d\n", x + 1, x + 2);
    gbuf = a->read(30);
    pos += gbuf.length();
    CPPUNIT_ASSERT_EQUAL(pos, a->tell());
    CPPUNIT_ASSERT_EQUAL((size_t) 30, gbuf.length());
    CPPUNIT_ASSERT_STRING_EQUAL(sbuf, gbuf);
    x += 2;
  }
  CPPUNIT_ASSERT_EQUAL(14, x);

  size_t savelen = a->tell();
  a->seek(a->tell(), SEEK_SET);
  a->puts("This is line15\n");
  a->seek(savelen, SEEK_SET);
  sbuf = "This is line15\n";
  gbuf = a->read(30);
  pos += gbuf.length();
  CPPUNIT_ASSERT_EQUAL(pos, a->tell());
  CPPUNIT_ASSERT_EQUAL((size_t) 15, gbuf.length());
  CPPUNIT_ASSERT_STRING_EQUAL(gbuf, sbuf);
}

void StreamTest :: getlineTest (void)
{
  String sbuf, gbuf;

  a->puts("This is line 1\n");
  a->puts("This is line 2\n");
  a->puts("This is line 3\n");
  a->puts("This is line 4\n");
  a->puts("This is line 5\n");
  a->puts("This is line 6\n");
  a->puts("This is line 7\n");
  a->puts("This is line 8\n");
  a->puts("This is line 9\n");
  a->puts("This is line10\n");
  a->puts("This is line11\n");
  a->puts("This is line12\n");
  a->puts("This is line13\n");
  a->seek(0, SEEK_SET);
  int x = 0;
  size_t pos = 0;
  while (a->tell() < a->length()) {
    ++x;
    sbuf = String::printf("This is line%2d\n", x);
    gbuf = a->getline();
    pos += gbuf.length();
    CPPUNIT_ASSERT_EQUAL(pos, a->tell());
    CPPUNIT_ASSERT_EQUAL((size_t) 15, gbuf.length());
    CPPUNIT_ASSERT_STRING_EQUAL(gbuf, sbuf);
  }
  CPPUNIT_ASSERT_EQUAL(13, x);
}

void StreamTest :: loadFileTest (void)
{
  const char *file = "/etc/passwd";

  FILE *f = NULL;
  f = fopen(file, "rb");
  if (f == NULL)
    return;

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buffer = (char*) malloc(size + 1);
  fread(buffer, 1, size, f);
  buffer[size] = 0;

  a->loadFile(file);

  CPPUNIT_ASSERT_EQUAL(size, a->length());
  CPPUNIT_ASSERT(*a == buffer);
  free(buffer);

  char buf[1024] = "";
  String gbuf;

  fseek(f, 0, SEEK_SET);
  while (fgets(buf, sizeof(buf), f) != NULL) {
    gbuf = a->getline(sizeof(buf) - 1);
    CPPUNIT_ASSERT_STRING_EQUAL(String(buf), gbuf);
  }
  fclose(f);

  struct stat s;
  stat(file, &s);
  CPPUNIT_ASSERT_EQUAL(a->length(), size_t(s.st_size));
}


void StreamTest :: writeFileFDTest (void)
{
  char fname[20] = "";
  strcpy(fname, ".stream-out-XXXXXX");
  int fd = mkstemp(fname);
  const char *file = "/etc/passwd";

  a->loadFile(file);
  CPPUNIT_ASSERT_EQUAL(a->writeFile(fd), 0);

  /* Verify that the written file matches the source file */

  FILE *f = NULL;
  f = fopen(file, "rb");
  if (f == NULL)
    return;

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buffer = (char*) malloc(size + 1);
  fread(buffer, 1, size, f);
  buffer[size] = 0;

  CPPUNIT_ASSERT_EQUAL(size, a->length());
  CPPUNIT_ASSERT(*a == buffer);
  free(buffer);

  char buf[1024] = "";
  String gbuf;

  fseek(f, 0, SEEK_SET);
  while (fgets(buf, sizeof(buf), f) != NULL) {
    gbuf = a->getline(sizeof(buf) - 1);
    CPPUNIT_ASSERT_STRING_EQUAL(String(buf), gbuf);
  }

  struct stat s;
  fstat(fd, &s);
  CPPUNIT_ASSERT_EQUAL(a->length(), size_t(s.st_size));
  stat(file, &s);
  CPPUNIT_ASSERT_EQUAL(a->length(), size_t(s.st_size));

  unlink(fname);
  fclose(f);
  close(fd);
}

void StreamTest :: writeFileTest (void)
{
  const char *writefile = "/tmp/bdlib";
  const char *file = "/etc/passwd";

  a->loadFile(file);
  CPPUNIT_ASSERT_EQUAL(a->writeFile(writefile), 0);

  /* Verify that the written file matches the source file */

  FILE *f = NULL;
  f = fopen(file, "rb");
  if (f == NULL)
    return;

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buffer = (char*) malloc(size + 1);
  fread(buffer, 1, size, f);
  buffer[size] = 0;

  CPPUNIT_ASSERT_EQUAL(size, a->length());
  CPPUNIT_ASSERT(*a == buffer);
  free(buffer);

  char buf[1024] = "";
  String gbuf;

  fseek(f, 0, SEEK_SET);
  while (fgets(buf, sizeof(buf), f) != NULL) {
    gbuf = a->getline(sizeof(buf) - 1);
    CPPUNIT_ASSERT_STRING_EQUAL(String(buf), gbuf);
  }

  struct stat s;
  int fd = open(writefile, O_RDONLY);
  fstat(fd, &s);
  CPPUNIT_ASSERT_EQUAL(a->length(), size_t(s.st_size));
  stat(file, &s);
  CPPUNIT_ASSERT_EQUAL(a->length(), size_t(s.st_size));

  unlink(writefile);
  close(fd);
  fclose(f);
}
