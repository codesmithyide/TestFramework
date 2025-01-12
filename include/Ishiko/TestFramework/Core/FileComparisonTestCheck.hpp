// SPDX-FileCopyrightText: 2000-2024 Xavier Leclercq
// SPDX-License-Identifier: BSL-1.0

#ifndef GUARD_ISHIKO_CPP_TESTFRAMEWORK_CORE_FILECOMPARISONTESTCHECK_HPP
#define GUARD_ISHIKO_CPP_TESTFRAMEWORK_CORE_FILECOMPARISONTESTCHECK_HPP

#include "TestCheck.hpp"
#include "TestContext.hpp"
#include <boost/filesystem.hpp>
#include <string>

namespace Ishiko
{

class FileComparisonTestCheck : public TestCheck
{
public:
    FileComparisonTestCheck();
    FileComparisonTestCheck(boost::filesystem::path outputFilePath, boost::filesystem::path referenceFilePath);
    static FileComparisonTestCheck CreateFromContext(const TestContext& context,
        const boost::filesystem::path& outputAndReferenceFilePath, TestContext::PathResolution path_resolution);
    static FileComparisonTestCheck CreateFromContext(const TestContext& context,
        const boost::filesystem::path& outputFilePath, const boost::filesystem::path& referenceFilePath,
        TestContext::PathResolution path_resolution);

    void run(Test& test, const char* file, int line) override;

    const boost::filesystem::path& outputFilePath() const;
    void setOutputFilePath(const boost::filesystem::path& path);
    const boost::filesystem::path& referenceFilePath() const;
    void setReferenceFilePath(const boost::filesystem::path& path);

    void addToJUnitXMLTestReport(JUnitXMLWriter& writer) const override;

private:
    boost::filesystem::path m_outputFilePath;
    boost::filesystem::path m_referenceFilePath;
    // We store this because we want to have the option of displaying it in test reports
    std::string m_firstDifferentLine;
};

}

#endif
