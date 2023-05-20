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
}
/******************************************************************************/
