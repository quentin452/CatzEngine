/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
TreeNode* TreeNode::first()
{
   auto node=this;
   while(auto left=node->left)node=left;
   return node;
}
TreeNode* TreeNode::last()
{
   auto node=this;
   while(auto right=node->right)node=right;
   return node;
}
TreeNode* TreeNode::next()
{
   auto node=this;
   if(auto right=node->right)
   {
      while(auto left=right->left)right=left;
      return right;
   }
    
   auto   parent=node->parent();
   while( parent && node==parent->right){node=parent; parent=node->parent();}
   return parent;
}
TreeNode* TreeNode::prev()
{
   auto node=this;
   if(auto left=node->left)
   {
      while(auto right=left->right)left=right;
      return left;
   }

   auto   parent=node->parent();
   while( parent && node==parent->left){node=parent; parent=node->parent();}
   return parent;
}
/******************************************************************************/
#define RED     0
#define BLACK   3

#define LEFT    3 // left  higher
#define RIGHT   2 // right higher
#define BALANCE 0
#define WEAK    3
#define WLEFT   1
#define WRIGHT  2

static inline void ReplaceNodeAsLeftChild (TreeNode* newnode, TreeNode* parent) {parent->left =newnode;}
static inline void ReplaceNodeAsRightChild(TreeNode* newnode, TreeNode* parent) {parent->right=newnode;}

static inline void ReplaceNode(TreeNode* oldnode, TreeNode* newnode, TreeNode* parent, TreeNode*& root)
{
   if(parent)
   {
      if(parent->left==oldnode)ReplaceNodeAsLeftChild (newnode, parent);
      else                     ReplaceNodeAsRightChild(newnode, parent);
   }else
   {
      root=newnode;
   }
}

static inline void RotateLeftAsLeftChild(TreeNode* node)
{
   auto right =node->right;
   auto parent=node->parent();
   node->right=right->left;
   if(right->left)right->left->parent(node);
   right->left=node;
   right->parent(parent);
   ReplaceNodeAsLeftChild(right, parent);
   node->parent(right);
}

static inline void RotateRightAsRightChild(TreeNode* node)
{
   auto left  =node->left;
   auto parent=node->parent();
   node->left =left->right;
   if(left->right)left->right->parent(node);
   left->right=node;
   left->parent(parent);
   ReplaceNodeAsRightChild(left, parent);
   node->parent(left);
}

static inline void RotateLeft(TreeNode* node, TreeNode*& root)
{
   auto right =node->right;
   auto parent=node->parent();
   node->right=right->left;
   if(right->left)right->left->parent(node);
   right->left=node;
   right->parent(parent);
   ReplaceNode(node, right, parent, root);
   node->parent(right);
}

static inline void RotateRight(TreeNode* node, TreeNode*& root)
{
   auto left  =node->left;
   auto parent=node->parent();
   node->left =left->right;
   if(left->right)left->right->parent(node);
   left->right=node;
   left->parent(parent);
   ReplaceNode(node, left, parent, root);
   node->parent(left);
}
/******************************************************************************/
TreeNode* _RBInsert(TreeNode* node, TreeNode* root)
{
   TreeNode *parent, *gparent;
   node->setTag<RED>();

   while((parent=node->parent()) && parent->tag()==RED)
   {
      gparent=parent->parent();

      if(parent==gparent->left)
      {
         TreeNode *uncle=gparent->right;
         if(uncle && uncle->tag()==RED)
         {
              uncle->setTag<BLACK>();
             parent->setTag<BLACK>();
            gparent->setTag<RED  >();
            node=gparent;
            continue;
         }

         if(parent->right==node)
         {
            TreeNode *tmp;
            RotateLeftAsLeftChild(parent);
            tmp=parent;
            parent=node;
            node=tmp;
         }

          parent->setTag<BLACK>();
         gparent->setTag<RED  >();
         RotateRight(gparent, root);
      }else
      {
         TreeNode *uncle=gparent->left;
         if(uncle && uncle->tag()==RED)
         {
              uncle->setTag<BLACK>();
             parent->setTag<BLACK>();
            gparent->setTag<RED  >();
            node=gparent;
            continue;
         }

         if(parent->left==node)
         {
            TreeNode *tmp;
            RotateRightAsRightChild(parent);
            tmp=parent;
            parent=node;
            node=tmp;
         }

          parent->setTag<BLACK>();
         gparent->setTag<RED  >();
         RotateLeft(gparent, root);
      }
   }

   root->setTag<BLACK>();
   return root;
}
inline TreeNode *RBRemover(TreeNode *node, TreeNode *parent, TreeNode *root)
{
   TreeNode *other;

   while((!node || node->tag()==BLACK) && node!=root)
   {
      if(parent->left==node)
      {
         other=parent->right;
         if(other->tag()==RED)
         {
            other ->setTag<BLACK>();
            parent->setTag<RED  >();
            RotateLeft(parent, root);
            other=parent->right;
         }
         if((!other->left || other->left->tag()==BLACK) && (!other->right || other->right->tag()==BLACK))
         {
            other->setTag<RED>();
            node=parent;
            parent=node->parent();
         }else
         {
            if(!other->right || other->right->tag()==BLACK)
            {
               TreeNode *o_left;
               if((o_left=other->left))o_left->setTag<BLACK>();
               other->setTag<RED>();
               RotateRightAsRightChild(other);
               other=parent->right;
            }
            other ->setTag(parent->tag());
            parent->setTag<BLACK>();
            if(other->right)other->right->setTag<BLACK>();
            RotateLeft(parent, root);
            node=root;
            break;
         }
      }else
      {
         other=parent->left;
         if(other->tag()==RED)
         {
            other ->setTag<BLACK>();
            parent->setTag<RED  >();
            RotateRight(parent, root);
            other=parent->left;
         }
         if((!other->left || other->left->tag()==BLACK) && (!other->right || other->right->tag()==BLACK))
         {
            other->setTag<RED>();
            node=parent;
            parent=node->parent();
         }else
         {
            if(!other->left || other->left->tag()==BLACK)
            {
               TreeNode *o_right;
               if((o_right=other->right))o_right->setTag<BLACK>();
               other->setTag<RED>();
               RotateLeftAsLeftChild(other);
               other=parent->left;
            }
            other ->setTag(parent->tag());
            parent->setTag<BLACK>();
            if(other->left)other->left->setTag<BLACK>();
            RotateRight(parent, root);
            node=root;
            break;
         }
      }
   }
   if(node)node->setTag<BLACK>();
   return root;
}
/******************************************************************************/
TreeNode* _AVLInsert(TreeNode* node, TreeNode* root)
{
   node->setTag<BALANCE>();
   for(TreeNode* parent=node->parent(); parent; node=parent, parent=node->parent())
   {
      auto tag=parent->tag();
      if(node==parent->left) // left child
      {
         if(tag==LEFT)
         {
            auto node_tag=node->tag();
            if(  node_tag==RIGHT)
            {
               auto tmp=node->right;
               auto tmp_tag=tmp->tag();
               RotateLeftAsLeftChild(node);
               parent->setTag((tmp_tag==LEFT ) ? RIGHT : BALANCE);
               node  ->setTag((tmp_tag==RIGHT) ? LEFT  : BALANCE);
               tmp   ->setTag<BALANCE>();
            }else
            {
               parent->setTag((node_tag==LEFT) ? BALANCE : LEFT );
               node  ->setTag((node_tag==LEFT) ? BALANCE : RIGHT);
            }
            RotateRight(parent, root);
            return root;
         }else
         if(tag==BALANCE)
         {
            parent->setTag<LEFT>();
         }else
         {
            parent->setTag<BALANCE>();
            return root;
         }
      }else // right child
      {                  
         if(tag==RIGHT)
         {
            auto node_tag=node->tag();
            if(  node_tag==LEFT)
            {
               auto tmp=node->left;
               auto tmp_tag=tmp->tag();
               RotateRightAsRightChild(node);
               parent->setTag((tmp_tag==RIGHT) ? LEFT  : BALANCE);
               node  ->setTag((tmp_tag==LEFT ) ? RIGHT : BALANCE);
               tmp   ->setTag<BALANCE>();
            }else
            {
               parent->setTag((node_tag==RIGHT) ? BALANCE : RIGHT);
               node  ->setTag((node_tag==RIGHT) ? BALANCE : LEFT );
            }
            RotateLeft(parent, root);
            return root;
         }else
         if(tag==BALANCE)
         {
            parent->setTag<RIGHT>();
         }else
         {
            parent->setTag<BALANCE>();
            return root;
         }
      }
   }
   return root;
}
inline TreeNode* AVLRemover(TreeNode* node, TreeNode *parent, TreeNode* root, Bool left_child)
{
   for(;;)
   {
      auto tag=parent->tag();
      if(left_child) // left child
      {
         if(tag==RIGHT)
         {
            auto sibling    =parent->right;
            auto sibling_tag=sibling->tag();
            if(  sibling_tag==LEFT)
            {
               auto tmp    =sibling->left;
               auto tmp_tag=tmp->tag();
               RotateRightAsRightChild(sibling);
               parent ->setTag((tmp_tag==RIGHT) ? LEFT  : BALANCE);
               sibling->setTag((tmp_tag==LEFT ) ? RIGHT : BALANCE);
               tmp    ->setTag<BALANCE>();
               node=tmp;
            }else
            {
               parent ->setTag((sibling_tag==BALANCE) ? RIGHT : BALANCE);
               sibling->setTag((sibling_tag==BALANCE) ? LEFT  : BALANCE);
               node=sibling;
            }

            RotateLeft(parent, root);
            if(sibling_tag==BALANCE)return root;
         }else
         if(tag==BALANCE)
         {
            parent->setTag<RIGHT>();
            return root;
         }else
         {
            parent->setTag<BALANCE>();
            node=parent;
         }
      }else // right child
      {
         if(tag==LEFT)
         {
            auto sibling    =parent->left;
            auto sibling_tag=sibling->tag();
            if(  sibling_tag==RIGHT)
            {
               auto tmp    =sibling->right;
               auto tmp_tag=tmp->tag();
               RotateLeftAsLeftChild(sibling);
               parent ->setTag((tmp_tag==LEFT ) ? RIGHT : BALANCE);
               sibling->setTag((tmp_tag==RIGHT) ? LEFT  : BALANCE);
               tmp    ->setTag<BALANCE>();
               node=tmp;
            }else
            {
               parent ->setTag((sibling_tag==BALANCE) ? LEFT  : BALANCE);
               sibling->setTag((sibling_tag==BALANCE) ? RIGHT : BALANCE);
               node=sibling;
            }

            RotateRight(parent, root);
            if(sibling_tag==BALANCE)return root;
         }else
         if(tag==BALANCE)
         {
            parent->setTag<LEFT>();
            return root;
         }else
         {
            parent->setTag<BALANCE>();
            node=parent;
         }
      }
      parent=node->parent();
      if(!parent)break;
      left_child=(node==parent->left);
   }
   return root;
}
/******************************************************************************/
TreeNode* _WAVLInsert(TreeNode* node, TreeNode* root)
{
   node->setTag<BALANCE>();
   for(TreeNode* parent=node->parent(); parent; node=parent, parent=node->parent())
   {
      auto tag=parent->tag();
      if(node==parent->left) // left child
      {
         if(tag==WLEFT)
         {
            auto node_tag=node->tag();
            if(  node_tag==WRIGHT)
            {
               auto tmp    =node->right;
               auto tmp_tag=tmp->parent_tag;
               RotateLeftAsLeftChild(node);
               parent->setTag((tmp_tag&WLEFT ) ? WRIGHT : BALANCE);
               node  ->setTag((tmp_tag&WRIGHT) ? WLEFT  : BALANCE);
               tmp   ->setTag<BALANCE>();
            }else
            {
               parent->setTag((node_tag& WLEFT) ? BALANCE : WLEFT );
               node  ->setTag((node_tag==WLEFT) ? BALANCE : WRIGHT);
            }

            RotateRight(parent, root);
            return root;
         }else
         if(tag==WRIGHT)
         {
            parent->setTag<BALANCE>();
            return root;
         }else
         {
            parent->setTag<WLEFT>();
            if(tag==WEAK)return root;
         }
      }else // right child
      {
         if(tag==WRIGHT)
         {
            auto node_tag=node->tag();
            if(  node_tag==WLEFT)
            {
               auto tmp    =node->left;
               auto tmp_tag=tmp->parent_tag;
               RotateRightAsRightChild(node);
               parent->setTag((tmp_tag&WRIGHT) ? WLEFT  : BALANCE);
               node  ->setTag((tmp_tag&WLEFT ) ? WRIGHT : BALANCE);
               tmp   ->setTag<BALANCE>();
            }else
            {
               parent->setTag((node_tag& WRIGHT) ? BALANCE : WRIGHT);
               node  ->setTag((node_tag==WRIGHT) ? BALANCE : WLEFT );
            }

            RotateLeft(parent, root);
            return root;
         }else
         if(tag==WLEFT)
         {
            parent->setTag<BALANCE>();
            return root;
         }else
         {
            parent->setTag<WRIGHT>();
            if(tag==WEAK)return root;
         }
      }
   }
   return root;
}
inline TreeNode* WAVLRemover(TreeNode* node, TreeNode *parent, TreeNode* root, Bool left_child)
{
   for(;;)
   {
      auto tag=parent->tag();
      if(left_child) // left child
      {
         if(tag==WRIGHT)
         {
            auto sibling    =parent->right;
            auto sibling_tag=sibling->tag();
            if(  sibling_tag==WEAK)
            {
               sibling->setTag<BALANCE>();
            }else
            {
               if(sibling_tag==WLEFT)
               {
                  auto tmp    =sibling->left;
                  auto tmp_tag=tmp->tag();
                  RotateRightAsRightChild(sibling);
                  parent ->setTag((tmp_tag&WRIGHT) ? WLEFT  : BALANCE);
                  sibling->setTag((tmp_tag&WLEFT ) ? WRIGHT : BALANCE);
                  tmp    ->setTag<WEAK>();
               }else
               {
                  parent ->setTag((sibling_tag==WRIGHT) ? BALANCE : WRIGHT);
                  sibling->setTag((sibling_tag==WRIGHT) ? WEAK    : WLEFT);
               }

               RotateLeft(parent, root);
               return root;
            }
         }else
         if(tag==WLEFT)
         {
            parent->setTag<BALANCE>();
         }else
         {
            parent->setTag<WRIGHT>();
            if(tag==BALANCE)return root;
         }
      }else // right child
      {
         if(tag==WLEFT)
         {
            auto sibling    =parent->left;
            auto sibling_tag=sibling->tag();
            if(  sibling_tag==WEAK)
            {
               sibling->setTag<BALANCE>();
            }else
            {
               if(sibling_tag==WRIGHT)
               {
                  auto tmp    =sibling->right;
                  auto tmp_tag=tmp->tag();
                  RotateLeftAsLeftChild(sibling);
                  parent ->setTag((tmp_tag&WLEFT ) ? WRIGHT : BALANCE);
                  sibling->setTag((tmp_tag&WRIGHT) ? WLEFT  : BALANCE);
                  tmp    ->setTag<WEAK>();
               }else
               {
                  parent ->setTag((sibling_tag==WLEFT) ? BALANCE : WLEFT );
                  sibling->setTag((sibling_tag==WLEFT) ? WEAK    : WRIGHT);
               }

               RotateRight(parent, root);
               return root;
            }
         }else
         if(tag==WRIGHT)
         {
            parent->setTag<BALANCE>();
         }else
         {
            parent->setTag<WLEFT>();
            if(tag==BALANCE)return root;
         }
      }
      node=parent;
      parent=node->parent();
      if(!parent)break;
      left_child=(node==parent->left);
   }
   return root;
}
/******************************************************************************/
template<typename Remover>
static inline TreeNode* Remove(TreeNode *node, TreeNode* root)
{
   Remover remover;
   TreeNode *child, *parent, *old=node;
   if(node->left && node->right)
   {
      TreeNode *tmp;
      node=node->right;
      while(tmp=node->left)node=tmp;
      child =node->right;
      parent=node->parent();
      remover.setColor(node->tag());
      if(child)child->parent(parent);
      if(parent==old)
      {
         ReplaceNodeAsRightChild(child, parent);
         parent=node;
         remover.setAsLeftChild(false);
      }else
      {
         ReplaceNodeAsLeftChild(child, parent);
         remover.setAsLeftChild(true);
      }

      tmp=old->parent();
      node->left =old->left;
      node->right=old->right;
      node->parent(tmp);
      node->setTag(old->tag());
      ReplaceNode(old, node, tmp, root);
      old->left->parent(node);
      if(old->right)old->right->parent(node);
   }else
   {
      if(!node->left)child=node->right;
      else           child=node->left;
      parent=node->parent();
      remover.setColor(node->tag());
      remover.setAsLeftChild(parent && parent->left==node);
      ReplaceNode(node, child, parent, root);
      if(child)child->parent(parent);
   }
   old->zero();
   return remover(child, parent, root);
}

TreeNode* _RBRemove(TreeNode* node, TreeNode* root)
{
   struct RemoverRB
   {
      Int  color;
      void setColor(Int c) {color=c;}
      void setAsLeftChild(Bool)C {}
      TreeNode* operator()(TreeNode* child, TreeNode* parent, TreeNode* root)
      {
         if(color==BLACK)return RBRemover(child, parent, root);
         else            return root;
      }
   };
   return Remove<RemoverRB>(node, root);
}

TreeNode* _AVLRemove(TreeNode* node, TreeNode* root)
{
   struct RemoverAVL
   {
      Bool flag;
      void setColor(Int)C {}
      void setAsLeftChild(Bool f) {flag=f;}
      TreeNode* operator()(TreeNode* child, TreeNode* parent, TreeNode* root)
      {
         if(parent)return AVLRemover(child, parent, root, flag);
         else      return root;
      }
   };
   return Remove<RemoverAVL>(node, root);
}

TreeNode* _WAVLRemove(TreeNode* node, TreeNode* root)
{
   struct RemoverWAVL
   {
      Bool flag;
      void setColor(Int)C {}
      void setAsLeftChild(Bool f) {flag=f;}
      TreeNode* operator()(TreeNode* child, TreeNode* parent, TreeNode* root)
      {
         if(parent)return WAVLRemover(child, parent, root, flag);
         else      return root;
      }
   };
   return Remove<RemoverWAVL>(node, root);
}
/******************************************************************************/
void _RBValidate(C TreeNode* node)
{
   if(node)
   {
      if(node->tag()==RED)
      {
         REP(2)if(TreeNode *child=node->child[i])DYNAMIC_ASSERT(child->tag()!=RED, "RB.validate");
      }
      REP(2)_RBValidate(node->child[i]);
   }
}
/******************************************************************************/
}
/******************************************************************************/
