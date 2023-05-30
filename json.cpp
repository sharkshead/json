/*

MIT License

Copyright (c) 2023 Graeme Elsworthy <github@sharkshead.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define next()  c = fgetc(in); if(c == '\n') lineNumber++
// #define next()  { c = fgetc(in); if(c == '\n') lineNumber++; printf("%c", c); fflush(stdout); }
#define skip()  while((c == ' ') || (c == '\n') || (c == '\r')) next()

#define TYPE_KEYWORD    0
#define TYPE_INTEGER    1
#define TYPE_FLOAT      2
#define TYPE_STRING     3
#define TYPE_MAP        4
#define TYPE_ARRAY      5

#define REQ_TYPE_KEY    0
#define REQ_TYPE_INDEX  1
#define REQ_TYPE_STAR   2

#define WITH_INDENT        true
#define WITHOUT_INDENT    false

#define MAX_STRING  10240

struct Attribute;

struct Object {
  int type;
  union {
    const char *typeKeyword;
    int typeInteger;
    double typeFloat;
    char *typeString;
    Attribute *attributeList;
    Object *objectList;
  };
  Object *next;
};

struct Attribute {
  char *key;
  Object *value;
  Attribute *next;
};

struct Request {
  int type;
  union {
    char *key;
    int index;
  };
  Request *next;
};

FILE *in;
int c;
int lineNumber;

Attribute *parseAttribute() {
  Object *parseObject(void);
  Attribute *a;
  char key[MAX_STRING];
  char *p;
  char close;

  a = new Attribute;

  skip();
  if((c != '"') && (c != '\'')) {
    printf("Attribute key is not a string, line %d\n", lineNumber);
    exit(1);
  }
  close = c;
  next();

  p = key;
  while(c != close) {
    if(c == EOF) {
      printf("EOF in attribute key, line %d\n", lineNumber);
      exit(1);
    }
    if(c == '\\') next();
    if(p < &key[sizeof(key) - 2]) *p++ = c;

    next();
  }
  *p = 0;
  next();

  skip();
  if(c != ':') {
    printf("Attribute missing colon, line %d\n", lineNumber);
    exit(1);
  }
  next();

  if(c == EOF) {
    printf("EOF in attribute value, line %d\n", lineNumber);
    exit(1);
  }
  p = (char *) malloc(strlen(key) + 1);
  strcpy(p, key);
  a->key = p;
  a->value = parseObject();
  a->next = (Attribute *) 0;

  return a;
}

bool isKeyword(const char *keyword) {
  const char *p;

  p = keyword;
  while(*p != 0) {
    if(c != *p) return false;
    next();
    p++;
  }

  return true;
}

Object *parseObject() {
  Object *o;
  Attribute *a;
  Attribute **ap;
  Object *so;
  Object **op;
  char string[MAX_STRING];
  char *p;
  int i;
  double f;
  double factor;
  bool negative;
  char close;

  o = new Object;

  skip();
  if(c == '{') {
    o->type = TYPE_MAP;
    o->attributeList = (Attribute *) 0;
    ap = &o->attributeList;

    next();
    skip();
    while(c != '}') {
      a = parseAttribute();
      *ap = a;
      if(a != (Attribute *) 0) ap = &a->next;
      skip();
      if(c == ',') {
        next();
        skip();
      }
    }
    next();
  } else if(c == '[') {
    o->type = TYPE_ARRAY;
    o->objectList = (Object *) 0;
    op =  &o->objectList;
    do {
      next();
      so = parseObject();
      *op = so;
      if(so != (Object *) 0) op = &so->next;
    } while(c == ',');
    skip();
    if(c != ']') {
      printf("Array not terminated correctly, line %d\n", lineNumber);
      exit(1);
    }
    next();
  } else if((c == '"') || (c == '\'')) {
    close = c;
    next();
    o->type = TYPE_STRING;
    p = string;
    while(c != close) {
      if(c == EOF) {
        printf("EOF in string value key, line %d\n", lineNumber);
        exit(1);
      }
      if(c == '\\') {
        next();
        if(p < &string[sizeof(string) - 3]) {
          *p++ = '\\';
          *p++ = c;
        }
      } else {
        if(p < &string[sizeof(string) - 2]) *p++ = c;
      }

      next();
    }
    *p = 0;
    next();
    p = (char *) malloc(strlen(string) + 1);
    strcpy(p, string);
    o->typeString = p;
  } else if(((c >= '0') && (c <= '9')) || (c == '-') || (c == '.')) {
    o->type = TYPE_INTEGER;
    i = 0;
    if(c == '-') {
      next();
      negative = true;
    } else {
      negative = false;
    }
    do {
      i = 10 * i + (c - '0');
      next();
    } while((c >= '0') && (c <= '9'));
    if(c == '.') {
      next();
      o->type = TYPE_FLOAT;
      f = i;
      factor = 0.1;
      while((c >= '0') && (c <= '9')) {
        f = f + (factor * (c - '0'));
        factor /= 10.0;
        next();
      }
      if(negative) o->typeFloat = -f;
      else o->typeFloat = f;
    } else {
      if(negative) o->typeInteger = -i;
      else o->typeInteger = i;
    }
  } else if((c == 'n') && isKeyword("null")) {
    o->type = TYPE_KEYWORD;
    o->typeKeyword = "null";
  } else if((c == 't') && isKeyword("true")) {
    o->type = TYPE_KEYWORD;
    o->typeKeyword = "true";
  } else if((c == 'f') && isKeyword("false")) {
    o->type = TYPE_KEYWORD;
    o->typeKeyword = "false";
  } else {
    if(c != -1) {
      printf("Object expected. Instead have '%c', line %d\n", c, lineNumber);
      exit(1);
    }
    delete(o);
    o = (Object *) 0;
  }

  return o;
}

Request *parseRequest(char *req) {
  Request *ret;
  Request *r;
  char *p;
  char *q;
  int n;
  char close;

  ret = (Request *) 0;
  r = (Request *) 0;

  p = req;
  while(*p != 0) {
    if(r == (Request *) 0) {
      ret = new Request;
      ret->next = (Request *) 0;
      r = ret;
    } else {
      r->next = new Request;
      r = r->next;
      r->next = (Request *) 0;
    }

    if(*p == '[') {
      // index or quoted key
      p++;
      while(*p == ' ') p++;
      if((*p >= '0') && (*p <= '9')) {
        q = p;
        while((*q >= '0') && (*q <= '9')) q++;
        r->type = REQ_TYPE_INDEX;
        r->index = atoi(p);
      } else if((*p == '"') || (*p == '\'')) {
        close = *p++;
        q = p;
        while((*q != 0) && (*q != close)) q++;
        n = q - p;
        q++;
        r->type = REQ_TYPE_KEY;
        r->key = (char *) malloc(n + 1);
        strncpy(r->key, p, n);
        r->key[n] = 0;
      } else if(*p == ']') {
        r->type = REQ_TYPE_STAR;
        q = p;
      } else {
        printf("Illegal index or quoted key, line %d\n", lineNumber);
        exit(1);
      }
      while(*q == ' ') q++;
      if(*q != ']') {
        printf("Unclosed index, line %d\n", lineNumber);
        exit(1);
      }
      q++;
    } else {
      // key
      if(*p == '.') p++;
      q = p;
      while((*q != 0) && (*q != '.') && (*q != '[')) q++;
      n = q - p;
      r->type = REQ_TYPE_KEY;
      r->key = (char *) malloc(n + 1);
      strncpy(r->key, p, n);
      r->key[n] = 0;
    }

    p = q;
  }

  return ret;
}

void printAttribute(Attribute *a, int indent) {
  void printObject(const char *, Object *, int, bool);
  int i;

  for(i = 0; i < indent; i++) printf("  ");
  printf("\"%s\": ", a->key);
  printObject((const char *) 0, a->value, indent, WITHOUT_INDENT);
}

void printObject(const char *what, Object *o, int indent, bool withIndent) {
  Attribute *a;
  Object *so;
  int i;

  if(o == (Object *) 0) return;

  if((indent == 0) && (what[0] != 0)) printf("\"%s\": ", what);

  if(withIndent)
    for(i = 0; i < indent; i++) printf("  ");

  switch(o->type) {
    case TYPE_KEYWORD:
      printf("%s", o->typeKeyword);
      break;

    case TYPE_INTEGER:
      printf("%d", o->typeInteger);
      break;

    case TYPE_FLOAT:
      printf("%0.3f", o->typeFloat);
      break;

    case TYPE_STRING:
      printf("\"%s\"", o->typeString);
      break;

    case TYPE_MAP:
      if(o->attributeList == (Attribute *) 0) {
        printf("{}");
      } else {
        printf("{\n");
        for(a = o->attributeList; a != (Attribute *) 0; a = a->next) {
          printAttribute(a, indent + 1);
          if(a->next != (Attribute *) 0) printf(",");
          printf("\n");
        }
        for(i = 0; i < indent; i++) printf("  ");
        printf("}");
      }
      break;

    case TYPE_ARRAY:
      if(o->objectList == (Object *) 0) {
        printf("[]");
      } else {
        printf("[\n");
        for(so = o->objectList; so != (Object *) 0; so = so->next) {
          printObject(what, so, indent + 1, WITH_INDENT);
          if(so->next != (Object *) 0) printf(",");
          printf("\n");
        }
        for(i = 0; i < indent; i++) printf("  ");
        printf("]");
      }
      break;

    default:
      break;
  }

  if(indent == 0) printf("\n");
}

void printDetail(const char *what, Object *o, Request *r, int indent, bool withIndent) {
  Object *so;
  Attribute *a;
  int n;
  bool doIndent;


  while(r != (Request *) 0) {
    switch(r->type) {
      case REQ_TYPE_KEY:
        if(o->type == TYPE_MAP) {
          for(a = o->attributeList; a != (Attribute *) 0; a = a -> next) {
            if(strcmp(a->key, r->key) == 0) {
              if(r->next == (Request *) 0) {
                printObject(what, a->value, indent, withIndent);
              } else {
                r = r->next;
                printDetail(what, a->value, r, indent, withIndent);
              }
              break;
            }
          }
        }
        break;

      case REQ_TYPE_INDEX:
        if(o->type == TYPE_ARRAY) {
          so = o->objectList;
          n = 0;
          while((so != (Object *) 0) && (n < r->index)) {
            so = so->next;
            n++;
          }
          if(so != (Object *) 0) {
            if(r->next == (Request *) 0) {
              printObject(what, so, indent, withIndent);
            } else {
              r = r->next;
              printDetail(what, so, r, indent, withIndent);
            }
          }
        }
        break;

      case REQ_TYPE_STAR:
        if(o->type == TYPE_ARRAY) {
          if(r->next != (Request *) 0) {
            for(so = o->objectList; so != (Object *) 0; so = so->next) {
              printDetail(what, so, r, indent, withIndent);
            }
          }
        }
        break;

      default:
        break;
    }

    r = r->next;
  }
}

void printObjectDetail(const char *what, Object *o, Request *r, int indent, bool withIndent) {
  Object *so;
  int n;

  if(o->type == TYPE_ARRAY) {
    if(r->type == REQ_TYPE_INDEX) {
      for(n = 0, so = o->objectList; so != (Object *) 0; so = so->next, n++) {
        if(r->index == n) {
          if(r->next == (Request *) 0) {
            printObject(what, so, 0, WITHOUT_INDENT);
          } else {
            printDetail(what, so, r, 0, WITHOUT_INDENT);
          }
          break;
        }
      }
    } else {
      for(so = o->objectList; so != (Object *) 0; so = so->next) {
        printDetail(what, so, r, 0, WITHOUT_INDENT);
      }
    }
  } else {
    printDetail(what, o, r, 0, WITHOUT_INDENT);
  }
}

int main(int ac, char **av) {
  Object *o;
  Request *r;
  char what[MAX_STRING];
  int n;

  if((ac > 1) && (strcmp(av[1], "-") != 0)) {
    if((in = fopen(av[1], "r")) == NULL) {
      perror(av[1]);
      return 1;
    }
  } else {
    in = stdin;
  }

  lineNumber = 1;
  next();
  while((o = parseObject()) != (Object *) 0) {
    if(ac > 2) {
      for(n = 2; n < ac; n++) {
        strcpy(what, av[n]);
        r = parseRequest(what);
        printObjectDetail(av[n], o, r, 0, WITHOUT_INDENT);
      }
    } else {
      what[0] = 0;
      printObject(what, o, 0, WITH_INDENT);
    }
  }

  if(ac > 1) fclose(in);

  return 0;
}
