/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkDICOMTagBasedSorter_h
#define mitkDICOMTagBasedSorter_h

#include "mitkDICOMDatasetSorter.h"
#include "mitkDICOMSortCriterion.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Sort DICOM datasets based on configurable tags.

  This class implements sorting of input DICOM datasets into multiple outputs
  as described in \ref DICOMITKSeriesGDCMReader_LoadingStrategy.

  The logic of sorting and splitting is most simple and most generic:

  1. Datasets will be put into different groups, if they differ in their value of specific tags (defined by AddDistinguishingTag())
    - there might be multiple distinguishing tags defined
    - tag values might be processed before comparison by means of TagValueProcessor (e.g. round to a number of decimal places)
  2. Each of the groups will be sorted by comparing their tag values using multiple DICOMSortCriterion
    - DICOMSortCriterion might evaluate a single tag (e.g. Instance Number) or multiple values (as in SortByImagePositionPatient)
    - only a single DICOMSortCriterion is defined for DICOMTagBasedSorter, because each DICOMSortCriterion holds a "secondary sort criterion", i.e. an application can define multiple tags for sorting by chaining \link DICOMSortCriterion DICOMSortCriteria \endlink
    - applications should make sure that sorting is always defined (to avoid problems with standard containers), e.g. by adding a comparison of filenames or instance UIDs as a last sorting fallback.

*/
class DICOMReader_EXPORT DICOMTagBasedSorter : public DICOMDatasetSorter
{
  public:

    /**
      \brief Processes tag values before they are compared.
      These classes could do some kind of normalization such as rounding, lower case formatting, etc.
    */
    class TagValueProcessor
    {
      public:
        /// \brief Implements the "processing".
        virtual std::string operator()(const std::string&) const = 0;
    };

    /**
      \brief Cuts a number after configured number of decimal places.
      An instance of this class can be used to avoid errors when comparing minimally different image orientations.
    */
    class CutDecimalPlaces : public TagValueProcessor
    {
      public:
        CutDecimalPlaces(unsigned int precision);
        virtual std::string operator()(const std::string&) const;
      private:
        unsigned int m_Precision;
    };

    mitkClassMacro( DICOMTagBasedSorter, DICOMDatasetSorter )
    itkNewMacro( DICOMTagBasedSorter )

    /**
      \brief Datasets that differ in given tag's value will be sorted into separate outputs.
    */
    void AddDistinguishingTag( const DICOMTag&, TagValueProcessor* tagValueProcessor = NULL );

    /**
      \brief Define the sorting criterion (which holds seconardy criteria)
    */
    void SetSortCriterion( DICOMSortCriterion::ConstPointer criterion );

    /**
      \brief A list of all the tags needed for processing (facilitates scanning).
    */
    virtual DICOMTagList GetTagsOfInterest();

    /**
      \brief Actually sort as described in the Detailed Description.
    */
    virtual void Sort();

    /**
      \brief Print configuration details into given stream.
    */
    virtual void PrintConfiguration(std::ostream& os, const std::string& indent = "") const;

  protected:

    /**
      \brief Helper struct to feed into std::sort, configured via DICOMSortCriterion.
    */
    struct ParameterizedDatasetSort
    {
      ParameterizedDatasetSort(DICOMSortCriterion::ConstPointer);
      bool operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right);
      DICOMSortCriterion::ConstPointer m_SortCriterion;
    };


    DICOMTagBasedSorter();
    virtual ~DICOMTagBasedSorter();

    DICOMTagBasedSorter(const DICOMTagBasedSorter& other);
    DICOMTagBasedSorter& operator=(const DICOMTagBasedSorter& other);

    /**
      \brief Helper for SplitInputGroups().
    */
    std::string BuildGroupID( DICOMDatasetAccess* dataset );

    typedef std::map<std::string, DICOMDatasetList> GroupIDToListType;

    /**
      \brief Implements the "distiguishing tags".
      To sort datasets into different groups, a long string will be built for each dataset. The string concatenates all tags and their respective values.
      Datasets that match in all values will end up with the same string.
    */
    GroupIDToListType SplitInputGroups();

    /**
      \brief Implements the sorting step.
      Relatively simple implementation thanks to std::sort and a parameterization via DICOMSortCriterion.
    */
    GroupIDToListType& SortGroups(GroupIDToListType& groups);

    DICOMTagList m_DistinguishingTags;
    typedef std::map<const DICOMTag, TagValueProcessor*>  TagValueProcessorMap;
    TagValueProcessorMap m_TagValueProcessor;

    DICOMSortCriterion::ConstPointer m_SortCriterion;
};

}

#endif
