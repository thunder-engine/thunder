#ifndef ASINGLETON
#define ASINGLETON

template <typename T>
class ASingleton {
public:
    static T                   *instance        () {
        if(!m_pInstance) {
            m_pInstance = new T;
        }
        return m_pInstance;
    }

    static void                 destroy         () {
        delete m_pInstance;
        m_pInstance = 0;
    }

    ASingleton                  () { m_pInstance = 0; }

private:
    static T *m_pInstance;

};

template <typename T>
T *ASingleton<T>::m_pInstance = 0;

#endif // ASINGLETON

