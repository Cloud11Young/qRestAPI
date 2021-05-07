// Qt includes
#include <QTest>

// qRestAPI includes
#include "qRestAPI.h"

#include <QMap>

// --------------------------------------------------------------------------
class qRestAPITester : public  QObject
{
  Q_OBJECT
private slots:
  void cleanup();
  void testqVariantMapFlattened_data();
  void testqVariantMapFlattened();
private:
  QVariantMap LastTestInputMap;
  QVariantMap LastTestOutputMap;
  QVariantMap LastTestExpectedMap;
};

// --------------------------------------------------------------------------
namespace
{
///
/// \brief Return an indented representation of a QVariant considering map and list.
///
/// \example
///
/// ```
/// a: 1
/// b:
///   b_a: 2-1
///   b_b: 2-2
///   b_c: 2-3
/// c:
///   0:
///     c_a_1: 3-1_1
///     c_b_1: 3-2_1
///     c_c_1: 3-3_1
///   1:
///     c_a_2: 3-1_3
///     c_b_2: 3-2_2
///     c_b_3: 3-2_3
/// ```
QString toString(const QVariant &value, int indent=0)
{
  QString output;
  bool isVariantList = value.canConvert<QVariantList>();
  bool isVariantMap = value.canConvert<QVariantMap>();
  if ((isVariantList || isVariantMap) && indent > 0)
    {
    output += "\n";
    }
  if (isVariantList)
    {
    QVariantList list = value.toList();
    for(int idx=0; idx < list.count(); ++idx)
      {
      output.append(QString("%1%2: ").arg(QString(indent, ' ')).arg(idx));
      output.append(toString(list.at(idx), indent + 2));
      }
    }
  else if (isVariantMap)
    {
    QVariantMap map = value.toMap();
    foreach(const QString& key, map.keys())
      {
      output.append(QString("%1%2: ").arg(QString(indent, ' ')).arg(key));
      output.append(toString(map.value(key), indent + 2));
      }
    }
  else
    {
    return value.toString() + "\n";
    }
  return output;
}
}

// --------------------------------------------------------------------------
void qRestAPITester::cleanup()
{
  if (QTest::currentTestFailed())
    {
    if (!this->LastTestOutputMap.isEmpty() && !this->LastTestExpectedMap.isEmpty())
      {
      qDebug().noquote().nospace()
          << QTest::currentTestFunction()
          << "\ninput:\n" << toString(this->LastTestInputMap)
          << "\noutput:\n" << toString(this->LastTestOutputMap)
          << "\nexpected:\n" << toString(this->LastTestExpectedMap);
      }
    }
  this->LastTestInputMap.clear();
  this->LastTestOutputMap.clear();
  this->LastTestExpectedMap.clear();
}

// --------------------------------------------------------------------------
void qRestAPITester::testqVariantMapFlattened_data()
{
  QTest::addColumn<QVariantMap>("input");
  QTest::addColumn<QVariantMap>("expected");

  {
  // input
  QVariantMap input;
  input["a"] = "1";
  input["b"] = "2";
  input["c"] = "3";

  QVariantMap expected = input;

  QTest::newRow("identity") << input << expected;
  }

  {
  // input
  QVariantMap inputSubmap;
  inputSubmap["b_a"] = "2-1";
  inputSubmap["b_b"] = "2-2";
  inputSubmap["b_c"] = "2-3";

  QVariantMap input;
  input["a"] = "1";
  input["b"] = inputSubmap;
  input["c"] = "3";

  // expected
  QVariantMap expected;
  expected["a"] = "1";
  expected["b.b_a"] = "2-1";
  expected["b.b_b"] = "2-2";
  expected["b.b_c"] = "2-3";
  expected["c"] = "3";

  QTest::newRow("map-of-map") << input << expected;
  }

  {
  // input
  QVariantMap inputSubmapForB;
  inputSubmapForB["b_a"] = "2-1";
  inputSubmapForB["b_b"] = "2-2";
  inputSubmapForB["b_c"] = "2-3";


  QVariantMap inputSubmap1ForC;
  inputSubmap1ForC["c_a_1"] = "3-1_1";
  inputSubmap1ForC["c_b_1"] = "3-2_1";
  inputSubmap1ForC["c_c_1"] = "3-3_1";

  QVariantMap inputSubmap2ForC;
  inputSubmap2ForC["c_a_2"] = "3-1_2";
  inputSubmap2ForC["c_b_2"] = "3-2_2";

  QVariantMap inputSubmap3ForC;
  inputSubmap3ForC["c_a_2"] = "3-1_3"; // key already specified in inputSubmap2ForC
  inputSubmap3ForC["c_b_3"] = "3-2_3";

  QVariantMap input;
  input["a"] = "1";
  input["b"] = inputSubmapForB;
  input["c"] = QVariantList() << inputSubmap1ForC << inputSubmap2ForC << inputSubmap3ForC;

  // expected
  QVariantMap expected;
  expected["a"] = "1";

  expected["b.b_a"] = "2-1";
  expected["b.b_b"] = "2-2";
  expected["b.b_c"] = "2-3";

  expected["c.c_a_1"] = "3-1_1";
  expected["c.c_b_1"] = "3-2_1";
  expected["c.c_c_1"] = "3-3_1";

//  expected["c.c_a_2"] = "3-1_2";
  expected["c.c_b_2"] = "3-2_2";

  expected["c.c_a_2"] = "3-1_3";
  expected["c.c_b_3"] = "3-2_3";

  QTest::newRow("map-with-list-of-maps") << input << expected;
  }
}

// --------------------------------------------------------------------------
void qRestAPITester::testqVariantMapFlattened()
{
  QFETCH(QVariantMap, input);
  QFETCH(QVariantMap, expected);

  QVariantMap output = qRestAPI::qVariantMapFlattened(input);

  this->LastTestInputMap = input;
  this->LastTestOutputMap = output;
  this->LastTestExpectedMap = expected;

  QCOMPARE(output, expected);
}

#define main qRestAPITest
QTEST_MAIN(qRestAPITester)
#undef main

#include "moc_qRestAPITest.cpp"
