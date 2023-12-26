#include <memory>
#include <string>

class BST;

class BSTNode {
 public:
 	BSTNode(int key);
 	BSTNode(int key, std::weak_ptr<BSTNode> parent);
 	bool IsLeaf() const;
 	bool IsMissingChild() const;
 	bool HasLeftChild() const;
 	bool HasRightChild() const;
 	void DeleteChild(std::shared_ptr<BSTNode> v);
 	void ReplaceChild(std::shared_ptr<BSTNode> v, std::shared_ptr<BSTNode> u);

 private:
  int key_;
  std::weak_ptr<BSTNode> parent_;
  std::shared_ptr<BSTNode> left_;
  std::shared_ptr<BSTNode> right_;

  friend BST;
}; // class BSTNode

class BST {
 public:
 	BST();

 	void Insert(int key);
 	bool Delete(int key);
 	bool Find(int key) const;
 	std::string JSON() const;
 	size_t size() const;
 	bool empty() const;
 	int DeleteMin();

 private:
	void DeleteLeaf(std::shared_ptr<BSTNode> currentNode);
	int DeleteMin(std::shared_ptr<BSTNode> currentNode);

 	std::shared_ptr<BSTNode> root_;
 	size_t size_;
}; // class BST