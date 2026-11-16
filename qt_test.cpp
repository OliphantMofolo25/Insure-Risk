#include <QtTest/QTest>
#include <QString>
#include <QObject>

class InsuranceTest : public QObject {
	Q_OBJECT

private slots: 

	void testAddition(){
		QCOMPARE(5+5, 10);
		}
	void testRiskCategory(){
		QString risk = "High";
		QCOMPARE(risk, QString("High"));
		}
	void testValidAge(){
		int age = 25;
		QVERIFY(age >= 18);
		}
	void testInvalidAge(){
		int age = -5;
		QVERIFY(age < 0);
		}
};

QTEST_APPLESS_MAIN(InsuranceTest)
#include "qt_test.moc"
