////////////////////////////////////////////////////////////////////////////////
// <@meta=.metablank>
////////////////////////////////////////////////////////////////////////////////
# ifndef TB_PORT_INCLUDED_
#  define TB_PORT_INCLUDED_

# include <unistd.h>

# include <atomic>
# include <chrono>
# include <iostream>
# include <vector>

namespace threadbone {

typedef long long Ver_t;
typedef size_t Exit_t;                                            // exit status

/*............................................................................*/
template<typename TChannel, size_t num>
class TbPort {

private:
  size_t m_num;
  std::atomic_flag m_busy_buf1;
  std::atomic_flag m_busy_buf2;
  std::vector<TChannel> m_buf0;       // буфер для ссылок в случае неудачи
  std::vector<TChannel> m_buf1;
  std::vector<TChannel> m_buf2;
  TChannel* m_buf1_data;
  TChannel* m_buf2_data;
  Ver_t m_wr_ver_buf1;
  Ver_t m_wr_ver_buf2;
  Ver_t m_wr_ver;
  Ver_t m_rd_ver;
  uint m_fetched;
  bool m_wr_idx;

public:
//  TbPort(size_t num) noexcept;
  TbPort() noexcept;
  TbPort(const TbPort& p) noexcept;             // конструктор копирования
  inline size_t size(void) const;
  inline Ver_t wr_ver(void) const;
  inline Ver_t rd_ver(void) const;
  Exit_t write(const std::vector<TChannel>& data, int64_t wait);
  Exit_t read(std::vector<TChannel>& data);
  const std::vector<TChannel>& fetch(bool& updated);
  Exit_t release(void);

private:
  inline Exit_t __write(const std::vector<TChannel>& data, bool idx);
};

/*............................................................................*/
template<typename TChannel, size_t num>
TbPort<TChannel, num>::TbPort() noexcept :
    m_num(num),
    m_buf0(1),
    m_buf1(num),
    m_buf2(num),
    m_buf1_data(m_buf1.data()),
    m_buf2_data(m_buf2.data()),
    m_wr_ver_buf1(0),
    m_wr_ver_buf2(0),
    m_wr_ver(0),
    m_rd_ver(0),
    m_fetched(0),
    m_wr_idx(false)
{

}

/*............................................................................*/
template<typename TChannel, size_t num>
TbPort<TChannel, num>::TbPort(const TbPort& p) noexcept
{
  m_num = p.m_num;
  m_buf0 = p.m_buf0;
  m_buf1 = p.m_buf1;
  m_buf2 = p.m_buf2;
  m_buf1_data = m_buf1.data();
  m_buf2_data = m_buf2.data();
  m_wr_ver_buf1 = p.m_wr_ver_buf1;
  m_wr_ver_buf2 = p.m_wr_ver_buf2;
  m_wr_ver = p.m_wr_ver;
  m_rd_ver = p.m_rd_ver;
  m_fetched = p.m_fetched;
  m_wr_idx = p.m_wr_idx;
}

/*............................................................................*/
template<typename TChannel, size_t num>
inline size_t TbPort<TChannel, num>::size(void) const {
  return m_num;
}

/*............................................................................*/
template<typename TChannel, size_t num>
inline Ver_t TbPort<TChannel, num>::wr_ver(void) const {
  return m_wr_ver;
}

/*............................................................................*/
template<typename TChannel, size_t num>
inline Ver_t TbPort<TChannel, num>::rd_ver(void) const {
  return m_rd_ver;
}

/*............................................................................*/
template<typename TChannel, size_t num>
inline Exit_t TbPort<TChannel, num>::__write(const std::vector<TChannel>& data,
    bool idx) {
  const TChannel* p_data = data.data();
  if (m_wr_idx == false) {
    if (!m_busy_buf1.test_and_set(std::memory_order_seq_cst)) {
      m_wr_ver++;
      m_wr_ver_buf1 = m_wr_ver;
      size_t i = 0; // близкое расположение к петле - выше скорость (~3 %)
      while (i < m_num) {
        // m_buf1[i] = data[i];      //  1.00 x (исходный)
        m_buf1_data[i] = p_data[i];  // ~2.55 x
        // m_buf1[i] = p_data[i];    // ~1.33 x
        // m_buf1_data[i] = data[i]; // ~1.33 x
        ++i;
      }
      m_busy_buf1.clear(std::memory_order_seq_cst);
      return EXIT_SUCCESS;
    }
  } else {
    if (!m_busy_buf2.test_and_set(std::memory_order_seq_cst)) {
      m_wr_ver++;
      m_wr_ver_buf2 = m_wr_ver;
      size_t i = 0; // близкое расположение к петле - выше скорость (~3 %)
      while (i < m_num) {
        // m_buf2[i] = data[i];      //  1.00 x (исходный)
        m_buf2_data[i] = p_data[i];  // ~2.55 x
        // m_buf2[i] = p_data[i];    // ~1.33 x
        // m_buf2_data[i] = data[i]; // ~1.33 x
        ++i;
      }
      m_busy_buf2.clear(std::memory_order_seq_cst);
      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
}

/*............................................................................*/
template<typename TChannel, size_t num>
Exit_t TbPort<TChannel, num>::write(const std::vector<TChannel>& data,
    int64_t wait) {
  if (data.size() != m_num) {
    std::cerr << ": wrong vector length (given: " << data.size() << ", must: "
        << m_num << ")" << std::endl;
    return EXIT_FAILURE;
  }
  // В ф-ции read возможен кратковременный захват обоих буферов для
  // определения буфера с наиболее свежими данными. Поэтому вызываем в
  // бесконечном цикле в надежде, что какой-нибудь из них скоро освободится.
  // Но запустим таймер, чтобы, в случае чего, принудительно выйти из цикла.
  int64_t usec = 0;
  auto start = std::chrono::high_resolution_clock::now();
  while (1) {
    // За один цикл нужно пройтись по обоим буферам. Казалось бы, можно
    // пройтись и по одному, т.к. в случае неудачи на следующем цикле все
    // равно пройдется по второму. Но если wait=0, второго цикла не будет.
    m_wr_idx = !m_wr_idx; // NOTE: менять перед вызовом __write
    if (__write(data, m_wr_idx) == EXIT_SUCCESS) {
      return EXIT_SUCCESS;
    }
    m_wr_idx = !m_wr_idx;
    if (__write(data, m_wr_idx) == EXIT_SUCCESS) {
      return EXIT_SUCCESS;
    }
    usec = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
    if (usec > wait) {
      std::cerr << ": wrong wait time (given: " << wait << ")" << std::endl;
      return EXIT_FAILURE;
    }
  }
}

/*............................................................................*/
template<typename TChannel, size_t num>
const std::vector<TChannel>& TbPort<TChannel, num>::fetch(bool& updated) {
  // Пытаемся захватить оба буфера, чтобы определить в каком из них
  // наиболее свежая версия данных. Если не получается, захватываем тот,
  // который свободен.
  if (m_fetched) {
    // Читатель уже подтянул один из буферов - второй блокировать запрещено
    updated = false;
    return m_buf0;
  }
  if (!m_busy_buf1.test_and_set(std::memory_order_seq_cst)) {
    if (!m_busy_buf2.test_and_set(std::memory_order_seq_cst)) {
      // Удалось захватить оба буфера.
      if (m_wr_ver_buf1 > m_rd_ver || m_wr_ver_buf2 > m_rd_ver) {
        if (m_wr_ver_buf1 > m_wr_ver_buf2) {
          // Как можно быстрее освобождаем 2-й буфер - в нем менее
          // свежие данные, нас он не интересует
          m_busy_buf2.clear(std::memory_order_seq_cst);
          m_fetched = 1;
          m_rd_ver = m_wr_ver_buf1;
          // освободить m_busy_buf1 методом release
          updated = true;
          return m_buf1;
        } else {
          // Как можно быстрее освобождаем 1-й буфер - в нем менее
          // свежие данные, нас он не интересует
          m_busy_buf1.clear(std::memory_order_seq_cst);
          m_fetched = 2;
          m_rd_ver = m_wr_ver_buf2;
          // освободить m_busy_buf2 методом release
          updated = true;
          return m_buf2;
        }
      } else {
        // Более свежих данных, чем уже были прочитаны, ни в одном
        // буфере нет. Освобождаем оба буфера.
        m_busy_buf1.clear(std::memory_order_seq_cst);
        m_busy_buf2.clear(std::memory_order_seq_cst);
        updated = false;
        return m_buf0;
      }
    }
    // Удалось захватить только 1-й буфер.
    if (m_wr_ver_buf1 > m_rd_ver) {
      m_fetched = 1;
      m_rd_ver = m_wr_ver_buf1;
      // освободить m_busy_buf1 методом release
      updated = true;
      return m_buf1;
    }
    m_busy_buf1.clear(std::memory_order_seq_cst);
    updated = false;
    return m_buf0;
  } else if (!m_busy_buf2.test_and_set(std::memory_order_seq_cst)) {
    // Удалось захватить только 2-й буфер.
    if (m_wr_ver_buf2 > m_rd_ver) {
      m_fetched = 2;
      m_rd_ver = m_wr_ver_buf2;
      // освободить m_busy_buf2 методом release
      updated = true;
      return m_buf2;
    }
    m_busy_buf2.clear(std::memory_order_seq_cst);
    updated = false;
    return m_buf0;
  }
  // Не удалось захватить ни одного буфер (маловероятно, т.к.
  // писатель в каждый момент времени захватывает не боее 1-го буфера)
  updated = false;
  return m_buf0;
}

/*............................................................................*/
template<typename TChannel, size_t num>
Exit_t TbPort<TChannel, num>::release(void) {
  if (m_fetched == 1) {
    m_busy_buf1.clear();
    m_fetched = 0;
    return EXIT_SUCCESS;
  } else if (m_fetched == 2) {
    m_busy_buf2.clear();
    m_fetched = 0;
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}

/*............................................................................*/
template<typename TChannel, size_t num>
Exit_t TbPort<TChannel, num>::read(std::vector<TChannel>& data) {
  // TChannel* p_data = data.data();
  if (data.size() != m_num) {
    std::cerr << ": wrong vector length (given: " << data.size() << ", must: "
        << m_num << ")" << std::endl;
    return EXIT_FAILURE;
  }
  bool updated = false;
  const std::vector<TChannel>& buf = fetch(updated);
  // const TChannel* p_buf = buf.data();
  if (updated) {
    size_t i = 0;
    while (i < m_num) {
      data[i] = buf[i];      // 1.00 x
      //  p_data[i] = p_buf[i];  // 1.00 x
      //  p_data[i] = buf[i];    // 1.00 x
      //  data[i] = p_buf[i];    // 1.00 x
      ++i;
    }
    // for (size_t i=0; i < m_num; ++i) {
    //     data[i] = buf[i];      //  1.00 x (исходный дизайн)
    //  // p_data[i] = p_buf[i];  // ~0.65 x
    //  // p_data[i] = buf[i];    // ~0.92 x
    //  // data[i] = p_buf[i];    // ~1.00 x
    // }
    return release(); //
  }
  return EXIT_FAILURE;
}

}  // namespace threadbone
# endif  // TB_PORT_INCLUDED_
