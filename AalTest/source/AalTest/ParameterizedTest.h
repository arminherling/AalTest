#pragma once

#include <AalTest/FailureBehavior.h>
#include <AalTest/Stringify.h>
#include <AalTest/TestBase.h>
#include <AalTest/TestResult.h>
#include <AalTest/TestRunnerOutputBase.h>

#include <memory>
#include <QString>

namespace AalTest
{
    template<typename TFunction, typename TData>
    class ParameterizedTest : public TestBase
    {
    public:
        ParameterizedTest(TFunction&& function, TData&& data, const QString& testName)
            : m_function{ function }
            , m_testName{ testName }
            , m_data{ data }
        {
        }

        QPoint writeHeader(const std::unique_ptr<TestRunnerOutputBase>& output, int currentTest, int totalTestCount) const override
        {
            return output->writeTestHeader(currentTest, totalTestCount, testName(), true);
        }

        void run(const std::unique_ptr<TestRunnerOutputBase>& output, int headerIndentation, int& currentTest) override
        {
            auto totalSubTestCount = m_data.size();
            auto testResult = TestResultKind::Passed;
            if (totalSubTestCount == 0)
                return;

            for (const auto& parameters : m_data)
            {
                QPoint resultPosition;
                try
                {
                    const auto stringifiedParameters = Sanitize(Stringify(parameters, true));
                    resultPosition = output->writeSubTestHeader(headerIndentation, currentTest, totalSubTestCount, stringifiedParameters);

                    std::apply(m_function, parameters);

                    output->updateTestResult(resultPosition, TestResultKind::Passed);
                    output->writeTestPassedMessage();
                    addResult(TestResultKind::Passed);
                }
                catch (SkipTestException& e)
                {
                    output->updateTestResult(resultPosition, TestResultKind::Skipped);
                    output->writeTestSkippedMessage(e);
                    addResult(TestResultKind::Skipped);
                }
                catch (FailedTestException& e)
                {
                    rerunTest(parameters);
                    output->updateTestResult(resultPosition, TestResultKind::Failed);
                    output->writeTestFailedMessage(e);
                    addResult(TestResultKind::Failed);
                }
                catch (ValueMismatchTestException& e)
                {
                    rerunTest(parameters);
                    output->updateTestResult(resultPosition, TestResultKind::Failed);
                    output->writeTestValueMismatchMessage(e);
                    addResult(TestResultKind::Failed);
                }
                catch (SnapshotCreatedTestException& e)
                {
                    rerunTest(parameters);
                    output->updateTestResult(resultPosition, TestResultKind::Failed);
                    output->writeSnapshotCreatedMessage(e);
                    addResult(TestResultKind::Failed);
                }
                currentTest++;
            }
        }

        void writeResult(const std::unique_ptr<TestRunnerOutputBase>& output, const QPoint& position) const override
        {
        }

        const QString& testName() const override
        {
            return m_testName;
        }

        int testCount() const override
        {
            return m_data.size();
        }

    private:
        template<typename TPara>
        void rerunTest(const TPara& parameters) const
        {
            if (g_aalTestFailureBehavior == FailureBehavior::DebugBreakAndRerun)
            {
                try
                {
                    TRIGGER_DEBUG_BREAK();
                    std::apply(m_function, parameters);
                }
                catch (...) {}
            }
        }

        TFunction m_function;
        TData m_data;
        QString m_testName;
    };
}
