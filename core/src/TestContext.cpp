// SPDX-FileCopyrightText: 2000-2024 Xavier Leclercq
// SPDX-License-Identifier: BSL-1.0

#include "TestContext.hpp"
#include "TestException.hpp"
#include "TestFrameworkErrorCategory.hpp"
#include <Ishiko/BasePlatform.hpp>
#include <Ishiko/FileSystem.hpp>
#include <Ishiko/Process.hpp>

using namespace Ishiko;

TestContext::TestContext()
    : m_parent(nullptr)
{
    m_dataDirectories["(default)"] = boost::filesystem::path();
    m_referenceDirectories["(default)"] = boost::filesystem::path();
    m_outputDirectories["(default)"] = boost::filesystem::path();
}

TestContext::TestContext(const TestContext* parent)
    : m_parent(parent)
{
}

const TestContext& TestContext::DefaultTestContext()
{
    static TestContext defaultContext;
    return defaultContext;
}

void TestContext::expand(boost::string_view variable, std::string& result, Error& error) const
{
    if (variable == "context.data")
    {
        result = getDataDirectory().string();
    }
    else if (variable == "context.output")
    {
        result = getOutputDirectory().string();
    }
    else if (variable == "context.reference")
    {
        result = getReferenceDirectory().string();
    }
    // TODO: error
}

boost::filesystem::path TestContext::getDataDirectory() const
{
    return getDataDirectory("(default)");
}

boost::filesystem::path TestContext::getDataDirectory(const std::string& id) const
{
    boost::filesystem::path result;
    std::map<std::string, boost::filesystem::path>::const_iterator it = m_dataDirectories.find(id);
    if (m_parent)
    {
        if (it != m_dataDirectories.end())
        {
            result = (m_parent->getDataDirectory(id) / it->second);
        }
        else
        {
            result = m_parent->getDataDirectory(id);
        }
    }
    else
    {
        if (it != m_dataDirectories.end())
        {
            return it->second;
        }
        else
        {
            throw TestException("getDataDirectory: no directory found with id " + id);
        }
    }
    return result;
}

boost::filesystem::path TestContext::getDataPath(const boost::filesystem::path& path) const
{
    return getDataDirectory() / path;
}

void TestContext::setDataDirectory(const boost::filesystem::path& path)
{
    setDataDirectory("(default)", path);
}

void TestContext::setDataDirectory(const std::string& id, const boost::filesystem::path& path)
{
    std::string expandedPath = CurrentEnvironment::ExpandVariablesInString(path.string(),
        CurrentEnvironment::SubstitutionFormat::DollarAndCurlyBrackets);

    m_dataDirectories[id] = expandedPath;
}

boost::filesystem::path TestContext::getReferenceDirectory() const
{
    return getReferenceDirectory("(default)");
}

boost::filesystem::path TestContext::getReferenceDirectory(const std::string& id) const
{
    boost::filesystem::path result;
    std::map<std::string, boost::filesystem::path>::const_iterator it = m_referenceDirectories.find(id);
    if (m_parent)
    {
        if (it != m_referenceDirectories.end())
        {
            result = (m_parent->getReferenceDirectory(id) / it->second);
        }
        else
        {
            result = m_parent->getReferenceDirectory(id);
        }
    }
    else
    {
        if (it != m_referenceDirectories.end())
        {
            return it->second;
        }
        else
        {
            throw TestException("getReferenceDirectory: no directory found with id " + id);
        }
    }
    return result;
}

boost::filesystem::path TestContext::getReferencePath(const boost::filesystem::path& path,
    PathResolution path_resolution) const
{
    if (path_resolution == PathResolution::none)
    {
        return getReferenceDirectory() / path;
    }
    else
    {
        const std::string& os_family = OS::Family();
        boost::filesystem::path platform_specific_path = path;
        if (platform_specific_path.has_extension())
        {
            platform_specific_path.replace_extension(os_family + platform_specific_path.extension().string());
        }
        else
        {
            platform_specific_path.replace_extension(os_family);
        }
        boost::filesystem::path proposed_reference_path = getReferenceDirectory() / platform_specific_path;
        if (!FileSystem::Exists(proposed_reference_path))
        {
            // TODO: make this more generic
            if ((os_family == "linux") || (os_family == "cygwin"))
            {
                boost::filesystem::path platform_specific_path = path;
                if (platform_specific_path.has_extension())
                {
                    platform_specific_path.replace_extension("unix" + platform_specific_path.extension().string());
                }
                else
                {
                    platform_specific_path.replace_extension("unix");
                }
                proposed_reference_path = getReferenceDirectory() / platform_specific_path;
                if (!FileSystem::Exists(proposed_reference_path))
                {
                    proposed_reference_path = getReferenceDirectory() / path;
                }
            }
            else
            {
                proposed_reference_path = getReferenceDirectory() / path;
            }
        }
        return proposed_reference_path;
    }
}

void TestContext::setReferenceDirectory(const boost::filesystem::path& path)
{
    setReferenceDirectory("(default)", path);
}

void TestContext::setReferenceDirectory(const std::string& id, const boost::filesystem::path& path)
{
    std::string expandedPath = CurrentEnvironment::ExpandVariablesInString(path.string(),
        CurrentEnvironment::SubstitutionFormat::DollarAndCurlyBrackets);

    m_referenceDirectories[id] = expandedPath;
}

boost::filesystem::path TestContext::getOutputDirectory() const
{
    return getOutputDirectory("(default)");
}

boost::filesystem::path TestContext::getOutputDirectory(const std::string& id) const
{
    boost::filesystem::path result;
    std::map<std::string, boost::filesystem::path>::const_iterator it = m_outputDirectories.find(id);
    if (m_parent)
    {
        if (it != m_outputDirectories.end())
        {
            result = (m_parent->getOutputDirectory(id) / it->second);
        }
        else
        {
            result = m_parent->getOutputDirectory(id);
        }
    }
    else
    {
        if (it != m_outputDirectories.end())
        {
            return it->second;
        }
        else
        {
            throw TestException("getOutputDirectory: no directory found with id " + id);
        }
    }
    return result;
}

boost::filesystem::path TestContext::getOutputDirectory(const std::string& id, Error& error) const
{
    boost::filesystem::path result;
    std::map<std::string, boost::filesystem::path>::const_iterator it = m_outputDirectories.find(id);
    if (m_parent)
    {
        if (it != m_outputDirectories.end())
        {
            result = (m_parent->getOutputDirectory(id) / it->second);
        }
        else
        {
            result = m_parent->getOutputDirectory(id, error);
        }
    }
    else
    {
        if (it != m_outputDirectories.end())
        {
            return it->second;
        }
        else
        {
            Fail(TestFrameworkErrorCategory::Value::generic_error, error);
        }
    }
    return result;
}

boost::filesystem::path TestContext::getOutputPath(const boost::filesystem::path& path) const
{
    return getOutputDirectory() / path;
}

void TestContext::setOutputDirectory(const boost::filesystem::path& path)
{
    setOutputDirectory("(default)", path);
}

void TestContext::setOutputDirectory(const std::string& id, const boost::filesystem::path& path)
{
    std::string expandedPath = CurrentEnvironment::ExpandVariablesInString(path.string(),
        CurrentEnvironment::SubstitutionFormat::DollarAndCurlyBrackets);

    m_outputDirectories[id] = expandedPath;
}

boost::filesystem::path TestContext::getApplicationPath() const
{
    boost::filesystem::path result;
    if (m_application_path)
    {
        result = *m_application_path;
    }
    else
    {
        if (m_parent)
        {
            result = m_parent->getApplicationPath();
        }
        else
        {
            throw TestException("getApplicationPath not set");
        }
    }
    return result;
}

void TestContext::setApplicationPath(const boost::filesystem::path& path)
{
    m_application_path = path;
}
