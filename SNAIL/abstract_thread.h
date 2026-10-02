
namespace SNAIL
{

class AbstractThread
{
public:
    static int run();

private:
    AbstractThread();
    ~AbstractThread();

    static int taskBody();

    virtual int init();
    virtual int loop(int initStatus);
};

}