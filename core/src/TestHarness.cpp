// SPDX-FileCopyrightText: 2000-2024 Xavier Leclercq
// SPDX-License-Identifier: BSL-1.0

#include "TestHarness.hpp"
#include "JUnitXMLWriter.hpp"
#include "TestProgressObserver.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <Ishiko/Errors.hpp>
#include <iostream>
#include <iomanip>
#include <memory>

using namespace Ishiko;

TestHarness::CommandLineSpecification::CommandLineSpecification()
{
    addNamedOption("context.data", {Ishiko::CommandLineSpecification::OptionType::single_value});
    addNamedOption("context.output", {Ishiko::CommandLineSpecification::OptionType::single_value});
    addNamedOption("context.reference", {Ishiko::CommandLineSpecification::OptionType::single_value});
    addNamedOption("context.application-path", {Ishiko::CommandLineSpecification::OptionType::single_value});
    addNamedOption("persistent-storage", {Ishiko::CommandLineSpecification::OptionType::single_value});
    addNamedOption("junit-xml-test-report", {Ishiko::CommandLineSpecification::OptionType::single_value});
}

TestHarness::Configuration::Configuration(const Ishiko::Configuration& configuration)
{
    const Ishiko::Configuration::Value* contextData = configuration.valueOrNull("context.data");
    if (contextData)
    {
        if (contextData->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_contextData = contextData->asString();
        }
        else
        {
            // TODO: error
        }
    }
    const Ishiko::Configuration::Value* contextOutput = configuration.valueOrNull("context.output");
    if (contextOutput)
    {
        if (contextOutput->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_contextOutput = contextOutput->asString();
        }
        else
        {
            // TODO: error
        }
    }
    const Ishiko::Configuration::Value* contextReference = configuration.valueOrNull("context.reference");
    if (contextReference)
    {
        if (contextReference->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_contextReference = contextReference->asString();
        }
        else
        {
            // TODO: error
        }
    }
    const Ishiko::Configuration::Value* context_application_path = configuration.valueOrNull("context.application-path");
    if (context_application_path)
    {
        if (context_application_path->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_application_path = context_application_path->asString();
        }
        else
        {
            // TODO: error
        }
    }
    const Ishiko::Configuration::Value* persistentStorage = configuration.valueOrNull("persistent-storage");
    if (persistentStorage)
    {
        if (persistentStorage->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_persistentStorage = persistentStorage->asString();
        }
        else
        {
            // TODO: error
        }
    }
    const Ishiko::Configuration::Value* junitXMLTestReport = configuration.valueOrNull("junit-xml-test-report");
    if (junitXMLTestReport)
    {
        if (junitXMLTestReport->type() == Ishiko::Configuration::Value::Type::string)
        {
            m_junitXMLTestReport = junitXMLTestReport->asString();
        }
        else
        {
            // TODO: error
        }
    }
}

const boost::optional<std::string>& TestHarness::Configuration::contextData() const
{
    return m_contextData;
}

const boost::optional<std::string>& TestHarness::Configuration::contextOutput() const
{
    return m_contextOutput;
}

const boost::optional<std::string>& TestHarness::Configuration::contextReference() const
{
    return m_contextReference;
}

const boost::optional<std::string>& TestHarness::Configuration::contextApplicatiponPath() const
{
    return m_application_path;
}

const boost::optional<std::string>& TestHarness::Configuration::persistentStoragePath() const
{
    return m_persistentStorage;
}

const boost::optional<std::string>& TestHarness::Configuration::junitXMLTestReport() const
{
    return m_junitXMLTestReport;
}

TestHarness::TestHarness(const std::string& title)
    : m_context(TestContext::DefaultTestContext()), m_topSequence(title, m_context),
    m_timestampOutputDirectory(true)
{
}

TestHarness::TestHarness(const std::string& title, const Configuration& configuration)
    : m_junitXMLTestReport(configuration.junitXMLTestReport()), m_context(TestContext::DefaultTestContext()),
    m_topSequence(title, m_context), m_timestampOutputDirectory(true)
{
    const boost::optional<std::string> contextDataPath = configuration.contextData();
    if (contextDataPath)
    {
        m_context.setDataDirectory(*contextDataPath);
    }
    const boost::optional<std::string> contextOutputPath = configuration.contextOutput();
    if (contextOutputPath)
    {
        m_context.setOutputDirectory(*contextOutputPath);
    }
    const boost::optional<std::string> contextReferencePath = configuration.contextReference();
    if (contextReferencePath)
    {
        m_context.setReferenceDirectory(*contextReferencePath);
    }
    const boost::optional<std::string> contextApplicationPath = configuration.contextApplicatiponPath();
    if (contextApplicationPath)
    {
        m_context.setApplicationPath(*contextApplicationPath);
    }
    const boost::optional<std::string> persistentStoragePath = configuration.persistentStoragePath();
    if (persistentStoragePath)
    {
        m_context.setOutputDirectory("persistent-storage", *persistentStoragePath);
    }
    if (m_context.getOutputDirectory() != "")
    {
        prepareOutputDirectory();
    }

}

int TestHarness::run()
{
    std::cout << "Test Suite: " << m_topSequence.name() << std::endl;

    int result = runTests();

    return result;
}

TestContext& TestHarness::context()
{
    return m_context;
}

TestSequence& TestHarness::tests()
{
    return m_topSequence;
}

void TestHarness::prepareOutputDirectory()
{
    if (m_timestampOutputDirectory)
    {
        boost::posix_time::ptime currentSecond = boost::posix_time::second_clock::universal_time();
        std::stringstream currentSecondStr;
        currentSecondStr << currentSecond.date().year()
            << std::setw(2) << std::setfill('0') << currentSecond.date().month().as_number()
            << std::setw(2) << std::setfill('0') << currentSecond.date().day() << "T"
            << std::setw(2) << std::setfill('0') << currentSecond.time_of_day().hours()
            << std::setw(2) << std::setfill('0') << currentSecond.time_of_day().minutes()
            << std::setw(2) << std::setfill('0') << currentSecond.time_of_day().seconds() << "Z";
        boost::filesystem::path newOutputDirectory = m_context.getOutputDirectory() / currentSecondStr.str();
        m_context.setOutputDirectory(newOutputDirectory.string());
    }
}

int TestHarness::runTests()
{
    try
    {
        std::shared_ptr<TestProgressObserver> progressObserver = std::make_shared<TestProgressObserver>(std::cout);
        m_topSequence.observers().add(progressObserver);

        std::cout << std::endl;
        m_topSequence.run();
        std::cout << std::endl;

        printDetailedResults();
        printSummary();
        if (m_junitXMLTestReport)
        {
            writeJUnitXMLTestReport(*m_junitXMLTestReport);
        }

        if (!m_topSequence.passed() && !m_topSequence.skipped())
        {
            return TestApplicationReturnCode::testFailure;
        }
        else
        {
            return TestApplicationReturnCode::ok;
        }
    }
    catch (...)
    {
        return TestApplicationReturnCode::exception;
    }
}

void TestHarness::printDetailedResults()
{
    m_topSequence.traverse(
        [](const Test& test) -> void
        {
            if (!test.passed())
            {
                const TestSequence* sequence = dynamic_cast<const TestSequence*>(&test);
                // Special case. If the sequence is empty we consider it to be a single unknown test case. If we didn't
                // do that this case would go unreported.
                if (!sequence || (sequence->size() == 0))
                {
                    std::cout << test.name() << " " << ToString(test.result()) << std::endl;
                }
            }
        });
}

void TestHarness::printSummary()
{
    size_t unknown = 0;
    size_t passed = 0;
    size_t passedButMemoryLeaks = 0;
    size_t exception = 0;
    size_t failed = 0;
    size_t skipped = 0;
    size_t total = 0;
    m_topSequence.getPassRate(unknown, passed, passedButMemoryLeaks, exception, failed, skipped, total);
    if (!m_topSequence.passed() && !m_topSequence.skipped())
    {
        std::cout << "Pass rate: " << std::fixed << std::setprecision(2) << (100 * (double)passed / (double)total)
            << "% (" << unknown << " unknown, "
            << passed << " passed, "
            << passedButMemoryLeaks << " passed but with memory leaks, "
            << exception << " threw exceptions, "
            << failed << " failed, "
            << skipped << " skipped, "
            << total << " total)" << std::endl;
        std::cout << std::endl;
        std::cout << "Test Suite FAILED!!!" << std::endl;
    }
    else
    {
        std::cout << "Pass rate: " << std::fixed << std::setprecision(2) << (100 * (double)passed / (double)total)
            << "% (" << unknown << " unknown, "
            << passed << " passed, "
            << passedButMemoryLeaks << " passed but with memory leaks, "
            << exception << " threw exceptions, "
            << failed << " failed, "
            << skipped << " skipped, "
            << total << " total)" << std::endl;
        std::cout << std::endl;
        std::cout << "Test Suite passed" << std::endl;
    }
}

void TestHarness::writeJUnitXMLTestReport(const std::string& path)
{
    // TODO: error handling
    Ishiko::Error error;

    // TODO: this is hardly correct. We should take the absolute path and create that. Or is this a feature?
    boost::filesystem::path reportPath = path;
    if (reportPath.has_parent_path())
    {
        boost::filesystem::create_directories(reportPath.parent_path());
    }

    size_t unknown = 0;
    size_t passed = 0;
    size_t passedButMemoryLeaks = 0;
    size_t exception = 0;
    size_t failed = 0;
    size_t skipped = 0;
    size_t total = 0;
    m_topSequence.getPassRate(unknown, passed, passedButMemoryLeaks, exception, failed, skipped, total);

    JUnitXMLWriter writer;
    writer.create(reportPath, error);
    writer.writeTestSuitesStart();
    writer.writeTestSuiteStart(total);

    m_topSequence.traverse(
        [&writer](const Test& test)
        {
            test.addToJUnitXMLTestReport(writer);
        });

    writer.writeTestSuiteEnd();
    writer.writeTestSuitesEnd();
}
