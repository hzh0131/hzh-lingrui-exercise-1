# lr-core-vector

用 C 语言手写一个 `std::vector`：你要做的事只有一件：根据 `include/vector.h` 的描述**把 `src/vector.c` 里的空壳函数填成能用的实现，让 `make test` 全绿**。

[vector 原理可视化](https://lingrui-studio.github.io/vector-playground/)

本题实现的是只存储 `int` 的教学版动态数组，具体约定以 [include/vector.h](include/vector.h) 为准。

## 目录结构

```
lr-core-vector/
├── readme.md         本文件
├── Makefile          构建脚本（不用改）
├── .gitignore        列举 git 需要忽视的文件
├── .clang-format     格式化要求
├── include/
│   └── vector.h      接口声明 + 函数注释（不用改，但要读懂）
├── src/
│   └── vector.c      ★ 你要实现的地方
└── tests/
    └── test.c        单元测试（不用改）
```

## 自检

补全 [include/vector.c](include/vector.c) 中的函数实现后，项目根目录运行 `make test`，若最后输出结果如下即表示你完成了本项目（本项目只有未完成和已完成两种状态，不存在中间值）：

```bash
== 通过 3393 项，失败 0 项 ==
全部通过，可以 commit & push 了
```

测试始终开启 ASan + UBSan，检测到错误会以失败状态退出，不提供关闭开关。常见错误会被直接指出来，例如：

```
ERROR: AddressSanitizer: heap-buffer-overflow on address 0x... at pc 0x...
READ of size 4 at 0x... thread T0
    #0 0x... in get src/vector.c:52
```

行号会直接指到出问题的那一行，看不懂的把完成代码和报错信息复制给 AI 问一下。

## 提交

- 完成下面的`实现思路`一节，简要说明你的各个函数是如何实现的，尤其注意内存管理的说明
- 把所有修改 commit 并 push 到 GitHub 上自己的 vector 仓库
- 在个人仓库的 Actions 页面手动触发一次自动评分工作流

## 实现思路
> 首先先讲一下下面这段注释。
- 第一，这段注释讲了data,end,cap各自的作用（这对后面size,capadity,pushback等函数的实现非常重要！在一开始做题还未熟悉时我就曾因为记错end和cap导致多走了很多弯路。），而且注意到这里end和cap都是开区间，因此后续在对缓冲区尾部进行操作时必须注意要不要“-1”。
- 第二，标明了零容量状态是时指针状态与此时对size() 和 capacity()两个函数的要求，这就避免了很多野指针出现与未定义行为。
- 第三，"所有函数要求 v 非 NULL；除 vector_init 外，v必须已初始化或处于全 NULL 状态。"隐含了一个要求：所有函数在开始之前都要对变量条件（输入进去的变量`特别是v`是否为NULL，能否进行后续操作）进行检验，要求我们必须记得初始化v。
- 第四，out不能是野指针，避免未定义行为。
   * data 指向缓冲区起点，end 指向最后一个元素的后一位，cap
   * 指向缓冲区末尾的后一位。 有效元素位于 [data, end)，剩余空间位于 [end, cap)。
   * 零容量状态下三个指针均为 NULL；此时 size() 和 capacity() 返回
   * 0，不能做空指针减法。 所有函数要求 v 非 NULL；除 vector_init 外，v
   * 必须已初始化或处于全 NULL 状态。
   * get、front、back、pop_back 的 out 必须指向可写的 int。
---

> 下面我将通过代码块与运行结果实例分别阐释一下我每个函数的实现思路
===================== 生命周期 =====================
### int vector_init(vector *v, size_t capacity)
```c
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
```
1. 我一开始看到`申请一块能装 capacity 个 int 的缓冲区，size()`时立刻就想到一轮练习中学到的calloc语句。在观察了vector结构体的结构以及data等各个变量的作用之后我确定了calloc语句申请到的内存的地址应该是传给data。有了大致思路之后，我继续看后续要求。
2. "调用前 v 不得持有尚未释放的缓冲区；字段无需预先清零，初始化不读取旧值"，这句是对程序员的提醒，是注意事项，无需写特定代码。
3. 看到`capacity 最大为 SIZE_MAX / sizeof(int)``capacity 为 0 时不分配内存，三个指针置为 NULL，返回 0`这两句要求之后，我想到了应该要在进行整体初始化之前对capacity的值进行判断，所以我在开头写了两个if语句，分capacity==0(两个=号！)和capacity>SIZE_MAX / sizeof(int)两种情况讨论。写完之后，我想起来了总要求中的`所有函数要求 v 非 NULL；`，于是我又写了一个if语句来判断v是否为NULL。
4. 做完准备工作时候，可以写calloc语句了。一轮的时候，我曾学习过calloc申请内存失败这个知识点，所以在这里我清楚意识到不能直接上来就把calloc申请到的内存的地址赋给data，而是应该先定义一个buf,判断是否申请内存成功，成功之后才赋给data。因为要求中说size()为0，因此此时end也保存着calloc申请到的指针，而cap则是data向后移capacity个sizeof(int)，即cap=buf + capacity。
>踩过的坑：
  - 没理解清楚v是结构体指针，直接把calloc申请到的内存的地址赋给了v而不是v->data。
  - 不知道'SIZE_MAX'的含义，忘记了size_t是无符号整数，导致写if语句的条件判断时计算混乱。
  - 忘记判断v是否为NULL，判断是`==`写成了`=`。

### void vector_destroy(vector *v)
```c
void vector_destroy(vector *v) {
    if(v==NULL)
    {return;}
    free (v->data);
    v->data=NULL;
    v->end=NULL;
    v->cap=NULL;
}
```
1. 与上一题一样，先判断v是否为NULL（类似操作若没有特殊情况下文不再重复写），然后把v->data free掉，最后再把v中三个指针都指向NULL
> 踩过的坑
  - 受上一题的影响，一开始是把整个v free掉而不是free掉v->data。  


===================== 容量与大小 =====================

### size_t size(const vector *v)
```c
size_t size(const vector *v) {
   if(v==NULL||v->data==NULL)
   return 0;
   return (size_t)(v->end-v->data);
}
```
1. 因为要统计元素数量，当然就要先判断data是否为NULL，若data为NULL，自然没有统计的必要了。
2. 现在我们手头上已经有end和data的地址，又有效元素位于 [data, end)，故元素的数量就是data与end之间这块内存大小中包含int类型内存的数量，即`(size_t)(v->end-v->data)`（这里要注意指针相减之后得到的是它们之间相隔的元素个数，元素类型与指针所保存的元素类型相同，这个值是一个有符号整数**ptrdiff_t**,所以要把它转化为size_t类型的变量）
>踩过的坑
  - 忘记判断v->data是否为NULL。
  - 乱用sizeof导致错误
  ```c
  return (size_t)((sizeof(v->end)-sizeof(v->data))/sizeof(int))
  ```

### size_t capacity(const vector *v) 
```c
size_t capacity(const vector *v) {
    if(v==NULL||v->data==NULL)
    return 0;
    return (size_t)(v->cap-v->data);
}
```
1. 因为要统计容量，所以依旧先判断v与v->data是否为NULL。
2. 后续操作与上一个类似。
>踩过的坑
  - 因为这个函数与上一个类似，上一个函数我在修改过程中已经认识到自己的错误，所以这个函数没有踩坑。

### int empty(const vector *v) 
```c
int empty(const vector *v) {
    if(size(v)==0)
    return 1;
    else 
    return 0;
}
```
1. 判断v是否为空，size()为0返回1，否则返回0。按要求做即可，简单
>踩过的坑
  - 无

===================== 元素访问 =====================
### int get(const vector *v, size_t index, int *out)   
```c
int get(const vector *v, size_t index, int *out) {
    if(v==NULL||v->data==NULL||out==NULL)
    return -1;
    
    if(index>=size(v))
    return -1;

    int *a=(int *)(v->data+index);
    *out=*a;
    
    return 0;
}
```
1. 根据总要求，先判断v,index,out是否为NULL。
2. `读取下标 index 处的元素，写入 *out`，因为index是一个无符号整数，因此可以满足指针运算的特殊情况，直接进行指针与index相加（即v->data+index），之后再分别对out与a解引用把a中的值赋给out即可。
3. `index >= size() 时返回 -1 且不写入 *out，成功返回 0`，要判断index是否超出大小，因此这一步放在上一步的前面。
4. `时间复杂度：O(1)`，不需要遍历data和end之间的所有元素，只需直接取出下标为index的元素即可。
>踩过的坑
  - 没有判断out是否为NULL；
  - 因忘记了指针的特殊运算规则，写出了下面的猎奇代码（修改时给自己都看笑了）
  ```c
  int *a=(int *)((size_t)(v->data)+index);
  ```
  - 没弄清楚接口约定，对out，v,v->data为NULL时应该返回-1还是0弄混淆了（看函数类型与函数目的）

### int set(vector *v, size_t index, int value)
```c
int set(vector *v, size_t index, int value) {
    if(v==NULL||v->data==NULL)
    return -1;
    
    if(index>=size(v))
    return -1;
    
    int *a=(int *)(v->data+index);
    *a=value;
    
    return 0;
}
```
1. `把下标 index 处的元素改成 value`，与上一题操作类似，只需要把` *out=*a;`改成` *a=value;`即可
2. `index >= size() 时返回 -1 且不修改任何元素，成功返回 0`多写一个if语句对size(v)进行判断。
3. `时间复杂度：O(1)`依旧不需遍历，只需直接取出index下标处的变量即可。
>踩过的坑
  - 与上一题思路类似，没有踩坑

### int front(const vector *v, int *out)
```c
int front(const vector *v, int *out) {
    if(v==NULL||empty(v)||out==NULL)
    return -1;
    
    *out=*(v->data);
    
    return 0;
}
```
1. 因为要`读取首元素，写入 *out`，所以依旧先进行v,v->data和out是否为NULL的判断
2. `empty(v) 时返回 -1 且不写入 *out，成功返回 0`，按要求写出if判断语句即可。
>踩过的坑
  - 比较简单，没有踩坑

### int back(const vector *v, int *out)
```c
int back(const vector *v, int *out) {
    if(v==NULL||out==NULL||empty(v))
    return -1;
    
    int *a=v->end-1;
    *out=*a;
    
    return 0;
}
```  
1. 因为要`读取末元素，写入 *out`且`empty(v) 时返回 -1 且不写入 *out，成功返回 0`,所以要检验v,out,是否为NULL以及v是否为空（如果v为空的话无法读取末元素，直接访问就会读取无效内存）。
2. 然后用int *a保存末元素地址，再与out同时解引用并把 *a的值赋给 *out即可。（**注意：因为end的一侧是开区间，所以不能用v->end表示末元素地址而是应该用v->end-1**）
>踩过的坑
  - 把v->end-1写成了v->end，导致越界访问出错。

===================== 修改器 ===================== 
### int push_back(vector *v, int value) 
```c
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
```
1. 首先看到`在尾部追加一个元素，size() 加一，容量不够时自动扩容`这是push_back函数最重要的功能，那么此时就要分两种情况讨论：1.容量不足（end==cap；）2.容量充足（1或以上）。
2. 根据上面所说的两种情况，我们要对不同情况做出合适的空间操作。容量充足时最简单，只需直接添加一个元素即可。但是如果end==cap容量不足，那这个时候就要分两种情况：capacity(v)=0即v为空以及capacity(v)!=0即v不为空，按`容量足够时容量不变；容量为 0 时增至 1，否则严格增至原容量的 2 倍`的要求来做。
3. 因此为了先解决复杂的容量不足的情况，很自然的写出if (v->end == v->cap)的语句，在if语句中，我为了方便理解，用oldsz,oldcap,newcap分别代表旧的v的元素数量，旧的v的容量以及新的容量。
4. 接着，就按照`容量足够时容量不变；容量为 0 时增至 1，否则严格增至原容量的 2 倍`的要求分`oldcap==0`与`oldcap!=0`两种情况写。（**这里要额外注意一点，因为size(v)不为0时容量要翻倍，所以要检验翻倍以后newcap是否会超过newcap > SIZE_MAX / sizeof(int)的界限，若是，要return -1,即 `两倍容量超过 SIZE_MAX / sizeof(int) 或分配失败时返回 -1，不修改 vector`**）
5. 然后就是用realloc语句来调整v的容量，这里有两个需要注意的地方：第一是要int *newbuf存扩容后的地址并判断是否为NULL**以防扩容失败**，第二是传进realloc里面的地址是v->data而不是v->cap！
6. 最后分别实现v->data = newbuf;v->end = newbuf + oldsz;v->cap = newbuf + newcap;即可完成容量不足时的扩容操作。
7. 做完了容量不足的情况，就只剩简单的容量充足情况，这时候只需end后移一位并把value赋给原来end下标位置的变量（不是end-1!）即可。
8. `时间复杂度：不扩容时 O(1)，扩容时 O(size())`,不扩容时，直接访问并修改v->end即可；而要扩容时要把原来全部 size() 个元素，逐个拷贝到新内存，因此时间复杂度为O(size())(**补充一点，如果计算均摊复杂度的话二倍扩容的方法就约为O(1)，因为虽然每一次扩容都要把上一次的所有元素重新拷贝一份，但是因为指数爆炸，相邻两次扩容之间可以进行的充足空间的push_back的次数增多，分摊下来就大约是O(1)。而如果是每次扩容都只增大1的话，那均摊复杂度就是O(n)，因为每一次都要重新拷贝之前的全部元素，这样性能很差，所以 std::vector 选择倍数扩容。**)
>踩过的坑
  - 一开始我因为没有弄清题意，犯了许多严重的错误，写出来的代码是这样的:
```c
  int push_back(vector *v, int value) {
    if(v==NULL)
    {return -1;}
    
    if(v->end==v->cap)
    {
        int *buf1=v->cap;  // 错误1：不能拿cap作为realloc的参数！realloc要传旧缓冲区起点 v->data，不是cap！
        int *bufplus1 = realloc(buf1, 1*sizeof(int)); // 错误2：扩容大小写错，不是固定1个int；而且容量0和非0的扩容策略不一样

        if(bufplus1=NULL)
        return -1;  
        else
        {
            v->cap=bufplus1;
            free(bufplus1); // 错误3:不能free新分配出来的内存！free之后这块内存直接失效
            free(buf1);     // 错误4：realloc成功时，旧内存已经被realloc自动释放，不要手动free旧指针
            return 0;
        }
    }
    else if(v->end+1<=v->cap)  // 多余判断，只要 end != cap 就有空位
    {
        v->end=v->end+1;        // 错误5：顺序错！应该先写值，再end++
        *(v->end)=value;
        return 0;
    }
    else
    {
        int *buf2=v->cap;
        int *bufplus2 = realloc(buf2, (size_t)(v->cap-v->data)*sizeof(int)); // 错误6：扩容大小等于原来大小，等于没扩容，没理解清楚realloc各部分参数的含义。
        if(bufplus2!=NULL)
        return -1;
        else
        {
            v->cap=bufplus2;
            free(bufplus2);//错误7：与上面一样
            free(buf2);//错误8：与上面一样
            return 0;
        }
    }
}
```  
  
  - 总之，一开始写的代码确实很烂，逻辑也有很多错误，不过好在弄清楚题意之后一点点改回来了，还加深了等于realloc语句的理解。

### int pop_back(vector *v, int *out)
```c
int pop_back(vector *v, int *out) {
    if(v==NULL||empty(v)||out==NULL)
    {
        return -1;
    }
    
    *out=*(v->end-1);
    v->end=v->end-1;

    return 0;
}   
```
1. 首先依旧是与上一题一样先判断v,empty(v)与out。`empty(v) 时返回 -1，不写入 *out 且不修改 vector；成功返回 0` 
2. `删除尾部元素并写入 *out，size() 减一，容量不变`，注意v->end-1即可
>踩过的坑
  - 用size(v)-1;删去元素（感觉这个做法很🍬，不知道当时自己是怎么想的）

### int reserve(vector *v, size_t new_cap)
``` c
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
```  

**注：因为在这个函数中capacity既是变量名又是函数名，这个名字重叠导致后面编译时一直报错，所以我不得不把capacity的变量名改为new_cap以便于区分，因此原来要求中的capacity我军用new_cap代替，而这个函数中出现的capacity则一直指的是函数名**
1. 先判断nem_cap `最大为 SIZE_MAX / sizeof(int)`以及v是否为NULL。`*超过上限或扩容失败时返回 -1，容量和元素都不变` 
2. 排除new_cap==0的情况，此时v为空，free掉data并把三个指针全部置NULL。
3. `扩容到至少能装 nem_cap 个元素，size() 和已有元素保持不变；容量只增不减`。为了方便理解，我用 old_cap保存了capacity(v)，；用size_t old_size保存了size(v)。又因为容量只增不减，所以要先讨论掉new_cap < old_cap的情况。
4. 做完这些准备工作之后，就可以用realloc进行容量扩增，做法与上面类似，这里不再赘述。
5. `成功（含 capacity 不大于当前容量、无需扩容）时返回 0` 
6. `时间复杂度：O(size())，不需要扩容时 O(1)`，因为要调用realloc函数，所以时间复杂度会不同，原因与上面类似，这里不赘述。
>踩过的坑
  - capacity == 0直接返回 -1。实际上这个函数允许 reserve (0)，含义是容量设为 0，不是非法，我不该直接拒绝。
  - v->end=v->data+capacity，改错了变量，应该改cap而不是改end

### shrink_to_fit(vector *v) 
```c
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
```
1. 目的是`收缩容量，使 capacity() == size()，元素和 size() 不变`，因此不仅要判断v是否为NULL，还要在`size() 为 0 时释放内存并把三个指针置为 NULL，返回 0`，以及在capacity=size时返回0.
2. 讨论完特殊情况之后，继续用realloc函数改变容量，新容量大小为size()*sizeof(int)。`失败时返回 -1，容量和元素都不变；成功返回 0`，具体操作与上文类似，不再赘述。
3. `时间复杂度：O(size())`，与上文类似，不再赘述。
>踩过的坑
  - free (v->data);的同时多进行了free(v->end);free(v->cap)：data、end、cap 都是**同一个堆缓冲区上的指针**，只是地址不同。v->end 和 v->cap 只是普通指针变量，它们不是独立分配的内存，不能 free！(**重复 free 会直接造成 double free，程序崩溃。**)

### void clear(vector *v) 
```c
void clear(vector *v) {
    if (v==NULL||v->data==NULL)
    {
        return;
    }

    v->end=v->data;
    return;

}  
```
1. `清空所有元素：size() 变成 0，容量和缓冲区保持不变`，因此依旧先判断v与v->data。
2. 让v->end=v->data即可，此时size（）为0，cap不变，容量与缓冲区不变。
>踩过的坑
  - 比较简单，没踩坑