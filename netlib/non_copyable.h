#ifndef NETLIB_NETLIB_NON_COPYABLE_H_
#define NETLIB_NETLIB_NON_COPYABLE_H_

namespace netlib
{

class NonCopyable // Class that disallow copy or assignment. Object semantics.
{
public:
	NonCopyable() = default; // Constructor
	~NonCopyable() = default; // Destructor

	NonCopyable(const NonCopyable&) = delete; // Copy constructor
	NonCopyable &operator=(const NonCopyable&) = delete; // Copy assignment operator
};

}

#endif // NETLIB_NETLIB_NON_COPYABLE_H_

class Parent;
using ParentPtr = std::shared_ptr<Parent>;

class Child: public netlib::NonCopyable
{
public:
	explicit Child(const ParentPtr &my_mom, const ParentPtr &my_dad):
		my_mom_(my_mom), my_dad_(my_dad)
	{}

private:
	std::weak_ptr<Parent> my_mom_;
	std::weak_ptr<Parent> my_dad_;
};
using ChildPtr = std::shared_ptr<Child>;

class Parent: public netlib::NonCopyable
{
public:
	Parent() {}
	void set_spouse(const ParentPtr &spouse)
	{
		my_spouse_ = spouse;
	}
	void AddChild(const ChildPtr &child)
	{
		my_children_.push_back(child);
	}

private:
	std::weak_ptr<Parent> my_spouse_;
	std::vector<ChildPtr> my_children_;
};

int main()
{
	ParentPtr mom(new Parent);
	ParentPtr dad(new Parent);
	mom->set_spouse(dad);
	dad->set_spouse(mom);
	{
		ChildPtr child(new Child(mom, dad));
		mom->AddChild(child);
		dad->AddChild(child);
	}
	{
		ChildPtr child(new Child(mom, dad));
		mom->AddChild(child);
		dad->AddChild(child);
	}
}
