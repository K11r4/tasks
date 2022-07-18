#include <bits/stdc++.h>

using namespace std;

#define bits __builtin_popcount
#define MP make_pair
#define PB push_back
#define debug(x) cout << #x << " is " << x << endl;

typedef long long ll;
// typedef __int128 bll;
typedef pair<int, int> pii;
typedef pair<long long, long long> pll;
typedef long double ld;

const ll noMod = 10000000000;

struct node {
  node *parent;
  node *left;
  node *right;
  ll val, sz, S, appMod, addMod, first, last;
  bool rev, inc, dec;

  // ll y;

  node(ll a) {
    // y = rand();
    parent = left = right = 0;
    val = a;
    S = a;
    sz = 1;
    appMod = noMod;
    addMod = 0;
    first = last = a;
    inc = dec = 1;
    rev = 0;
  }
};

ll getSz(node *T) {
  if (T)
    return T->sz;
  return 0;
}

ll getSum(node *T) { return T ? T->S : 0; }

node *p(node *T) {
  if (T)
    return T->parent;
  return 0;
}

node *g(node *T) { return p(p(T)); }

void reverse(node *T) {
  if (!T)
    return;
  T->rev ^= 1;
}

void addX(node *T, ll x) {
  if (!T)
    return;
  if (T->appMod != noMod)
    T->appMod += x;
  else
    T->addMod += x;
}

void appX(node *T, ll x) {
  if (!T)
    return;
  T->addMod = 0;
  T->appMod = x;
}

void push(node *T) {
  if (!T)
    return;

  if (T->rev) {
    swap(T->first, T->last), swap(T->left, T->right), swap(T->dec, T->inc);
    reverse(T->left);
    reverse(T->right);
    T->rev = 0;
  }

  if (T->appMod != noMod) {
    T->val = T->first = T->last = T->appMod;
    T->S = T->sz * T->appMod;
    T->dec = T->inc = 1;
    appX(T->left, T->appMod);
    appX(T->right, T->appMod);

  } else if (T->addMod != 0) {
    T->S += T->sz * T->addMod;
    T->val += T->addMod;
    T->first += T->addMod;
    T->last += T->addMod;
    addX(T->left, T->addMod);
    addX(T->right, T->addMod);
  }

  T->appMod = noMod;
  T->addMod = 0;
}

void recalc(node *T) {
  if (!T)
    return;
  push(T);
  push(T->left);
  push(T->right);

  T->sz = getSz(T->right) + getSz(T->left) + 1;
  T->S = getSum(T->left) + getSum(T->right) + T->val;

  T->first = T->left ? T->left->first : T->val;
  T->last = T->right ? T->right->last : T->val;

  bool rdec = 1, rinc = 1, ldec = 1, linc = 1;

  if (T->left) {
    if (!T->left->inc or T->val < T->left->last)
      linc = 0;
    if (!T->left->dec or T->val > T->left->last)
      ldec = 0;
  }

  if (T->right) {
    if (!T->right->inc or T->val > T->right->first)
      rinc = 0;
    if (!T->right->dec or T->val < T->right->first)
      rdec = 0;
  }

  T->dec = ldec and rdec;
  T->inc = linc and rinc;
}

void pushHard(node *T) {
  if (T) {
    push(T);
    push(T->left);
    push(T->right);
  }
}

node *find_k(node *T, int k) {
  if (!T)
    return 0;
  push(T);
  push(T->left);
  push(T->right);
  int L = getSz(T->left);
  if (L == k)
    return T;
  else if (L > k)
    return find_k(T->left, k);
  else
    return find_k(T->right, k - L - 1);
}

node *find_ma(node *T) {
  if (!T)
    return 0;
  push(T);
  push(T->left);
  push(T->right);
  if (!T->right)
    return T;
  else
    return find_ma(T->right);
}

void rotateL(node *T) {
  node *P = p(T);
  node *R = T->right;
  pushHard(P);
  pushHard(T);
  pushHard(R);
  if (P != 0) {
    if (P->left == T)
      P->left = R;
    if (P->right == T)
      P->right = R;
  }

  node *tmp = R->left;
  R->left = T;
  T->right = tmp;
  R->parent = P;
  T->parent = R;
  if (T->right)
    T->right->parent = T;

  recalc(T);
  recalc(R);
  recalc(P);
}

void rotateR(node *T) {
  node *P = p(T);
  node *L = T->left;
  pushHard(P);
  pushHard(T);
  pushHard(L);
  if (P != 0) {
    if (P->left == T)
      P->left = L;
    if (P->right == T)
      P->right = L;
  }

  node *tmp = L->right;
  L->right = T;
  T->left = tmp;
  L->parent = P;
  T->parent = L;
  if (T->left)
    T->left->parent = T;

  recalc(T);
  recalc(L);
  recalc(P);
}

void splay(node *T) {
  while (p(T) != 0) {
    if (T == p(T)->left) {
      if (g(T) == 0) {
        rotateR(p(T));
      } else if (g(T)->left == p(T)) {
        rotateR(g(T));
        rotateR(p(T));
      } else {
        rotateR(p(T));
        rotateL(p(T));
      }
    } else {
      if (g(T) == 0) {
        rotateL(p(T));
      } else if (g(T)->right == p(T)) {
        rotateL(g(T));
        rotateL(p(T));
      } else {
        rotateL(p(T));
        rotateR(p(T));
      }
    }
  }
}

node *merge(node *T1, node *T2) {
  if (!T1)
    return T2;
  if (!T2)
    return T1;
  node *tmp = find_ma(T1);
  splay(tmp);
  pushHard(tmp);
  push(T2);
  tmp->right = T2;
  T2->parent = tmp;
  recalc(tmp);
  return tmp;
}

pair<node *, node *> split(node *T, int k) {
  if (!T)
    return {0, 0};
  push(T);
  if (k >= T->sz)
    return {T, 0};
  if (k == 0)
    return {0, T};

  node *tmp = find_k(T, k);
  splay(tmp);
  pushHard(tmp);
  node *L = tmp->left;
  L->parent = 0;
  tmp->left = 0;
  recalc(tmp);
  return {L, tmp};
}

ll find1(node *T) {
  if (!T)
    return 0;
  push(T);
  push(T->right);
  push(T->left);
  if (T->dec)
    return T->sz;

  bool suff = !T->right or (T->val >= T->right->first and T->right->dec);
  bool pref = !T->left or (T->left->last >= T->val);
  if (!suff)
    return find1(T->right);
  else if (!pref)
    return 1 + getSz(T->right);
  else
    return find1(T->left) + 1 + getSz(T->right);
}

int find2(node *T, ll x) {
  if (!T)
    return 0;
  push(T);
  if (x >= T->first)
    return getSz(T);
  if (x >= T->val)
    return find2(T->left, x) + 1 + getSz(T->right);
  else
    return find2(T->right, x);
}

ll find3(node *T) {
  if (!T)
    return 0;
  push(T);
  push(T->right);
  push(T->left);
  if (T->inc)
    return T->sz;

  bool suff = !T->right or (T->val <= T->right->first and T->right->inc);
  bool pref = !T->left or (T->left->last <= T->val);
  if (!suff)
    return find3(T->right);
  else if (!pref)
    return 1 + getSz(T->right);
  else
    return find3(T->left) + 1 + getSz(T->right);
}

int find4(node *T, ll x) {
  if (!T)
    return 0;
  push(T);
  if (T->first >= x)
    return getSz(T);
  if (x <= T->val)
    return find4(T->left, x) + 1 + getSz(T->right);
  else
    return find4(T->right, x);
}

void print(node *T) {
  if (!T)
    return;
  push(T);
  print(T->left);
  cout << T->val << " ";
  print(T->right);
}

node *nextP(node *T) {
  if (!T)
    return T;
  push(T);
  int k = getSz(T) - find1(T);
  if (k == 0) {
    reverse(T);
  } else {
    pair<node *, node *> res = split(T, k);
    int l = getSz(res.second) - find2(res.second, res.first->last) - 1;
    pair<node *, node *> pref1 = split(res.first, k - 1);
    pair<node *, node *> suff1 = split(res.second, l + 1);
    pair<node *, node *> suff2 = split(suff1.first, l);
    node *pref = merge(pref1.first, suff2.second);
    node *suff = merge(merge(suff2.first, pref1.second), suff1.second);
    reverse(suff);
    T = merge(pref, suff);
  }

  return T;
}

node *prevP(node *T) {
  if (!T)
    return T;
  push(T);
  int k = getSz(T) - find3(T);
  if (k == 0) {
    reverse(T);
  } else {
    pair<node *, node *> res = split(T, k);
    int l = getSz(res.second) - find4(res.second, res.first->last) - 1;
    pair<node *, node *> pref1 = split(res.first, k - 1);
    pair<node *, node *> suff1 = split(res.second, l + 1);
    pair<node *, node *> suff2 = split(suff1.first, l);
    node *pref = merge(pref1.first, suff2.second);
    node *suff = merge(merge(suff2.first, pref1.second), suff1.second);
    reverse(suff);
    T = merge(pref, suff);
  }

  return T;
}
