/******************************************************************************/
struct TreeNode
{
   TreeNode *left, *right;
   UIntPtr   parent_tag;

   TreeNode* parent(           )C {return (TreeNode*)(parent_tag&~3);}
   void      parent(TreeNode* p)  {parent_tag&=3; parent_tag|=UIntPtr(p);}

   TreeNode* first();
   TreeNode*  last();
   TreeNode*  prev();
   TreeNode*  next();
 C TreeNode* first()C {return ConstCast(T).first();}
 C TreeNode*  last()C {return ConstCast(T). last();}
 C TreeNode*  prev()C {return ConstCast(T). prev();}
 C TreeNode*  next()C {return ConstCast(T). next();}

   static TreeNode* First(TreeNode *node) {return node ? node->first() : null;}
   static TreeNode*  Last(TreeNode *node) {return node ? node-> last() : null;}
   static TreeNode*  Prev(TreeNode *node) {return node ? node-> prev() : null;}
   static TreeNode*  Next(TreeNode *node) {return node ? node-> next() : null;}

#if !EE_PRIVATE
private:
#endif
   Int     tag(                              )C {return parent_tag&  3;}
   void setTag(Int t                         )  {       parent_tag&=~3; parent_tag|=t;}
   void setTag(std::integral_constant<Int, 3>)  {       parent_tag|= 3;}
   void setTag(std::integral_constant<Int, 0>)  {       parent_tag&=~3;}
   template<Int t> void setTag() {setTag(std::integral_constant<Int, t>());}
};
/******************************************************************************/
TreeNode*   _RBInsert(TreeNode* node, TreeNode* root);
TreeNode*  _AVLInsert(TreeNode* node, TreeNode* root);
TreeNode* _WAVLInsert(TreeNode* node, TreeNode* root);
TreeNode*   _RBRemove(TreeNode* node, TreeNode* root);
TreeNode*  _AVLRemove(TreeNode* node, TreeNode* root);
TreeNode* _WAVLRemove(TreeNode* node, TreeNode* root);
/******************************************************************************/
T2(Node, Key) struct BSTree
{
   Node*  root()C {return (Node*)                _root ;}
   Node* first()C {return (Node*)TreeNode::First(_root);}
   Node*  last()C {return (Node*)                _last ;} // TreeNode:: Last(_root)

   Node* find(C Key& value)C
   {
      Node *node=root();
      while(node)
      {
         if(compare(value, key(*node)))node=(Node*)node->left ;else
         if(compare(key(*node), value))node=(Node*)node->right;else
            return node;
      }
      return null;
   }

   void validate()C
   {
      DYNAMIC_ASSERT(_last==TreeNode::Last(_root), "BSTree.last");
      if(C TreeNode *node=_last)while(C TreeNode *prev=node->prev())
      {
       C Key &p=key(*(Node*)prev);
       C Key &n=key(*(Node*)node);
         DYNAMIC_ASSERT(compare(p, n) // p<n
                    || !compare(n, p) // !(n<p) = n>=p = p<=n   Check this in case compare is "<" but we need "<=", this will solve this
                    , "BSTree.order");
         node=prev;
      }
   }

   BSTree(C Key& (&key)(C Node &node), Bool (&compare)(C Key &a, C Key &b)) : key(key), compare(compare), _root(null), _last(null) {}

protected:
   void root(TreeNode* r) {_root=r;}

   void insert(Node* node)
   {
    C Key       &node_key=  key(*node);
      TreeNode** link    =&_root;
      Node*      parent  =  null;
      while(*link)
      {
         parent=(Node*)(*link);
         if(compare(node_key, key(*parent)))link=&(parent->left );
         else                               link=&(parent->right);
      }
      node->left=node->right=null;
      node->parent(parent);
     *link=node;

      if(!_last
      || !compare(node_key, key(*(Node*)_last)))_last=node;
   }
   Bool insertUnique(Node* node) // no duplicate keys
   {
    C Key       &node_key=  key(*node);
      TreeNode** link    =&_root;
      Node*      parent  =  null;
      while(*link)
      {
         parent=(Node*)(*link);
         if(compare(node_key, key(*parent)))link=&(parent->left );else
         if(compare(key(*parent), node_key))link=&(parent->right);else
            return false;
      }
      node->left=node->right=null;
      node->parent(parent);
     *link=node;

      if(!_last
      || !compare(node_key, key(*(Node*)_last)))_last=node;

      return true;
   }
   void removing(TreeNode &node)
   {
      if(&node==_last)_last=node.prev();
   }

private:
   static_assert(std::is_convertible<Node*, TreeNode*>::value, "'Node' must extend from 'TreeNode'");
 C Key&     (&key    )(C Node &node);
   Bool     (&compare)(C Key &a, C Key &b);
   TreeNode *_root, *_last;
};
/******************************************************************************/
T2(Node, Key) struct RBTree : BSTree<Node, Key>
{
   RBTree(C Key& (&key)(C Node &node), Bool (&compare)(C Key &a, C Key &b)) : BSTree(key, compare) {}

   void insert(Node* node)
   {
      super::insert(node);
      root(_RBInsert(node, root()));
   }
   void insertUnique(Node* node)
   {
      if(super::insertUnique(node))root(_RBInsert(node, root()));
   }
   void remove(Node* node)
   {
      if(node)
      {
         removing(*node);
         root(_RBRemove(node, root()));
      }
   }
};
/******************************************************************************/
T2(Node, Key) struct AVLTree : BSTree<Node, Key>
{
   AVLTree(C Key& (&key)(C Node &node), Bool (&compare)(C Key &a, C Key &b)) : BSTree(key, compare) {}

   void insert(Node* node)
   {
      super::insert(node);
      root(_AVLInsert(node, root()));
   }
   void insertUnique(Node* node)
   {
      if(super::insertUnique(node))root(_AVLInsert(node, root()));
   }
   void remove(Node* node)
   {
      if(node)
      {
         removing(*node);
         root(_AVLRemove(node, root()));
      }
   }
};
/******************************************************************************/
T2(Node, Key) struct WAVLTree : BSTree<Node, Key>
{
   WAVLTree(C Key& (&key)(C Node &node), Bool (&compare)(C Key &a, C Key &b)) : BSTree(key, compare) {}

   void insert(Node* node)
   {
      super::insert(node);
      root(_WAVLInsert(node, root()));
   }
   void insertUnique(Node* node)
   {
      if(super::insertUnique(node))root(_WAVLInsert(node, root()));
   }
   void remove(Node* node)
   {
      if(node)
      {
         removing(*node);
         root(_WAVLRemove(node, root()));
      }
   }
};
/******************************************************************************/
