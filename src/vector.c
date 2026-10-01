/* vector.c —— 你要实现的地方 */
#include <stdlib.h>
#include "vector.h"

int vector_init(vector *v, size_t capacity) 
{
    if(v==NULL)
    return -1;
   
    if(capacity==0)
    {
        v->data=NULL;
        v->end=NULL;
        v->cap=NULL;
        return 0;
    }
    if(capacity>(SIZE_MAX/sizeof(int)))
    {
        v->data=NULL;
        v->end=NULL;
        v->cap=NULL;
        return -1; 
    }
    
    int *buf = calloc(capacity, sizeof(int));
    if(buf==NULL)
    {
        v->data=NULL;
        v->end=NULL;
        v->cap=NULL;
        return -1; 
    }
    
    v->data = buf;
    v->end = buf;
    v->cap = buf + capacity;
    

    return 0;
}

void vector_destroy(vector *v) {
    if(v==NULL)
    {return;}
    free (v->data);
    v->data=NULL;
    v->end=NULL;
    v->cap=NULL;
}

size_t size(const vector *v) {
   if(v==NULL||v->data==NULL)
   return 0;
   return (size_t)(v->end-v->data);
}

size_t capacity(const vector *v) {
    if(v==NULL||v->data==NULL)
    return 0;
    return (size_t)(v->cap-v->data);
}

int empty(const vector *v) {
    if(size(v)==0)
    return 1;
    else 
    return 0;
}

int get(const vector *v, size_t index, int *out) {
    if(v==NULL||v->data==NULL||out==NULL)
    return -1;
    
    if(index>=size(v))
    return -1;

    int *a=(int *)(v->data+index);
    *out=*a;
    
    return 0;
}

int set(vector *v, size_t index, int value) {
    if(v==NULL||v->data==NULL)
    return -1;
    
    if(index>=size(v))
    return -1;
    
    int *a=(int *)(v->data+index);
    *a=value;
    
    return 0;
}

int front(const vector *v, int *out) {
    if(v==NULL||empty(v)||out==NULL)
    return -1;
    
    *out=*(v->data);
    
    return 0;
}

int back(const vector *v, int *out) {
    if(v==NULL||out==NULL||empty(v))
    return -1;
    
    int *a=v->end-1;
    *out=*a;
    
    return 0;
}

int push_back(vector *v, int value) {
    if (v == NULL)
        return -1;

    if (v->end == v->cap)
    {
        size_t oldsz = size(v); 
        size_t oldcap = capacity(v);
        size_t newcap;
        if (oldcap == 0)
        {
            newcap = 1;
        }
        else
        {
            newcap = 2 * oldcap;
        }

        if (newcap > SIZE_MAX / sizeof(int))
        {
            return -1;
        }
        int *newbuf = realloc(v->data, newcap * sizeof(int));
        if (newbuf == NULL)
        {
            return -1;
        }
        v->data = newbuf;
        v->end = newbuf + oldsz;
        v->cap = newbuf + newcap;
    }
    *(v->end) = value;
    v->end += 1;
    return 0;
}
    
int pop_back(vector *v, int *out) {
    if(v==NULL||empty(v)||out==NULL)
    {
        return -1;
    }
    
    *out=*(v->end-1);
    v->end=v->end-1;

    return 0;
}

int reserve(vector *v, size_t new_cap) {
    if (v == NULL)
        return -1;
    if (new_cap > SIZE_MAX / sizeof(int))
        return -1;

    // reserve(0) 需要释放内存，全部置NULL，满足测试用例
    if (new_cap == 0) {
        free(v->data);
        v->data = NULL;
        v->end = NULL;
        v->cap = NULL;
        return 0;
    }

    size_t old_cap = capacity(v);
    size_t old_size = size(v);

    if (new_cap <= old_cap) {
        return 0;
    }

    int *new_buf = realloc(v->data, new_cap * sizeof(int));
    if (new_buf == NULL)
    {
        return -1;
    }
    v->data = new_buf;
    v->end = new_buf + old_size;
    v->cap = new_buf + new_cap;
    return 0;
}


int shrink_to_fit(vector *v) {
    if (v == NULL) 
    {
        return -1;
    }
    size_t sz = size(v);
    size_t cap = capacity(v);

    if (cap == sz) 
    {
        return 0;
    }

    if (sz == 0) 
    {
        free(v->data);
        v->data = NULL;
        v->end = NULL;
        v->cap = NULL;
        return 0;
    }

    int *newbuf = realloc(v->data, sz * sizeof(int));
    if (newbuf == NULL) 
    {
        return -1;
    }

    v->data = newbuf;
    v->end  = newbuf + sz;
    v->cap  = newbuf + sz;

    return 0;
}

void clear(vector *v) {
    if (v==NULL||v->data==NULL)
    {
        return;
    }

    v->end=v->data;
    return;

}
