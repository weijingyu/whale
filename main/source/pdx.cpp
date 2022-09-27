// pdx.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "pdx.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <iomanip>

#include <log.h>



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


namespace whale {
    inline String getIdFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.attribute("ID").value();
    }

    inline String getShortNameFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.child_value("SHORT-NAME");
    }

    inline String getXsiTypeFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.attribute("xsi:type").value();
    }


    inline String getIdRefFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.attribute("ID-REF").value();
    }
    inline String getDocRefFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.attribute("DOCREF").value();
    }
    inline String getDocTypeFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.attribute("DOCTYPE").value();
    }

    inline String getTIFromXml(const pugi::xml_node& xmlNode) {
        return xmlNode.child("VT").attribute("TI").value();
    }

    String getDescriptionFromXml(const pugi::xml_node& xmlNode) {
        String description;
        for (const auto& p : xmlNode.child("DESC").children()) {
            description += p.first_child().value();
        }
        return description;
    }

    Map<String, int> ParentRefType{
        { "PROTOCOL-REF", 0 },
        { "FUNCTIONAL-GROUP-REF", 1 },
        { "BASE-VARIANT-REF", 2 },
        { "ECU-SHARED-DATA-REF", 3}
    };

    PDX PDX::s_instance;

    // ----- Start: Class Reference -----
    Reference::Reference(const pugi::xml_node& node)
        : idRef(getIdRefFromXml(node)),
        docRef(getDocRefFromXml(node)),
        docType(getDocTypeFromXml(node))
    {
    }

    Reference::Reference(String id, String doc, String type)
        : idRef(std::move(id)),
        docRef(std::move(doc)),
        docType(std::move(type))
    {
    }
    // ----- End: Class Reference -----

    // ----- Start: ComParamRef -----
    ComParamRef::ComParamRef(const pugi::xml_node& node) : Reference(node)
    {
        m_value = node.child("VALUE").text().as_int();
        m_protocolSNRef = node.child("PROTOCOL-SNREF").attribute("SHORT-NAME").value();
    }
    // ----- End: ComParamRef -----

    // ----- Start: BasicInfo -----
    BasicInfo::BasicInfo(const pugi::xml_node& node)
    {
        m_id = node.attribute("ID").value();
        m_shortName = node.child_value("SHORT-NAME");
        m_longName = node.child_value("LONG-NAME");
        //m_description = getDescriptionFromXml(node);
    }
    String BasicInfo::id() const
    {
        return m_id;
    }
    String BasicInfo::shortName() const
    {
        return m_shortName;
    }
    // ----- End: BasicInfo -----

    // ----- Start: PhysicalDimension -----
    PhysicalDimension::PhysicalDimension(const pugi::xml_node& node)
        : BasicInfo(node)
    {
        m_lengthExp = node.child("LENGTH-EXP").text().as_int();
        m_timeExp = node.child("TIME-EXP").text().as_int();
    }
    // ----- End: PhysicalDimension -----

    // ----- Start: Unit -----
    Unit::Unit(const pugi::xml_node& node, const Map<String, Ref<PhysicalDimension>>& physDimensionMap)
        : BasicInfo(node)
    {
        m_displayName = node.child("DISPLAY-NAME").text().as_string();
        m_factorSiToUnit = node.child("FACTOR-SI-TO-UNIT").text().as_uint();
        m_offsetSiToUnit = node.child("OFFSET-SI-TO-UNIT").text().as_uint();

        if (const auto& phdRef = node.child("PHYSICAL-DIMENSION-REF")) {
            auto id = getIdRefFromXml(phdRef);
            auto doc = getDocRefFromXml(phdRef);
            if (physDimensionMap.count(id)) {
                m_physicalDimension = physDimensionMap.at(id);
            }
            else {
                m_physicalDimension = PDX::get().getPhysicalDimensionByDocAndId(doc, id);
            }
        }
    }
    // ----- End: Class Unit -----

    // ----- Start: DiagCodedType -----
    DiagCodedType::DiagCodedType(const pugi::xml_node& node)
    {
        String xsiType = node.attribute("xsi:type").value();
        if (xsiType == "STANDARD-LENGTH-TYPE") {
            m_xsiType = DiagCodedType_E::StandardLength;
        }
        else if (xsiType == "MIN-MAX-LENGTH-TYPE") {
            m_xsiType = DiagCodedType_E::StandardLength;
        }
        else if (xsiType == "LEADING-LENGTH-INFO-TYPE") {
            m_xsiType = DiagCodedType_E::StandardLength;
        }
        else if (xsiType == "PARAM-LENGTH-INFO-TYPE") {
            m_xsiType = DiagCodedType_E::StandardLength;
        }   

        m_baseDataType = node.attribute("BASE-DATA-TYPE").value();

        if (const auto& temp = node.attribute("BASE-TYPE-ENCODING")) {
            m_baseTypeEncoding = temp.value();
        }
        if (const auto& temp = node.attribute("IS-HIGHLOW-BYTE-ORDER")) {
            m_isHighLowByteOrder = temp.as_bool();
        }
        if (const auto& temp = node.attribute("TERMINATION")) {
            m_isHighLowByteOrder = temp.value();
        }

        for (auto& child : node.children()) {
            if (!strcmp(child.name(),"BIT-LENGTH")) {
                m_bitLength = child.text().as_uint();
            }
            if (!strcmp(child.name(), "BIT-MASK")) {
                m_bitMask = child.value();
            }
            if (!strcmp(child.name(), "MAX-LENGTH")) {
                m_maxLength = child.text().as_uint();
            }
            if (!strcmp(child.name(), "MIN-LENGTH")) {
                m_minLength = child.text().as_uint();
            }
            if (!strcmp(child.name(), "LENGTH-KEY-REF")) {
                m_lengthKeyRef = getIdRefFromXml(child);
            }
        }
    }
    // ----- End: DiagCodedType -----

    // ----- Start: DopBase -----
    DopBase::DopBase(const pugi::xml_node& node) : BasicInfo(node)
    {
        m_isVisible = node.attribute("IS-VISIBLE").as_bool();
    }

    // ----- Start: DataObjectProp -----
    DataObjectProp::DataObjectProp(const pugi::xml_node& node)
        : DopBase(node)
    {
        m_compuMethod = CompuMethod(node.child("COMPU-METHOD"));
        m_diagCodedType = DiagCodedType(node.child("DIAG-CODED-TYPE"));
        m_physicalType = PhysicalType(node.child("PHYSICAL-TYPE"));
        m_internalConstr = InternalConstr(node.child("INTERNAL-CONSTR"));

        if (const auto& unitRef = node.child("UNIT-REF")) {
            m_unitRef = Reference(unitRef);
        }
    }

    void DataObjectProp::dereference(const Map<String, Ref<Unit>>& unitMap)
    {
        if (m_unitRef.has_value()) {
            auto id = m_unitRef->idRef;
            if (unitMap.count(id)) {
                m_unit = unitMap.at(id);
            }
            else {
                auto doc = m_unitRef->docRef;
                m_unit = PDX::get().getUnitByDocAndId(doc, id);
                if (!m_unit) {
                    WH_ERROR("No unit found!");
                }
            }
        }
    }

    DataObjectProp::PhysicalType::PhysicalType(const pugi::xml_node& node)
    {
        m_baseDataType = node.attribute("BASE-DATA-TYPE").value();

        if (m_baseDataType == "A_UINT32") {
            if (const auto& temp = node.attribute("DISPLAY-RADIX")) {
                m_dispayRadix = temp.value();
            }
        }

        if (m_baseDataType == "A_FLOAT32" || m_baseDataType == "A_FLOAT64") {
            if (const auto& temp = node.child("PRECISION")) {
                m_floatPrecision = temp.text().as_uint();
            }
        }
    }

    DataObjectProp::CompuMethod::CompuMethod(const pugi::xml_node& node)
    {
        m_category = node.child_value("CATEGORY");

        if (m_category == "TEXTTABLE") {
            for (const auto& ttcs : node.child("COMPU-INTERNAL-TO-PHYS").child("COMPU-SCALES").children()) {
                m_textTableCompuScales.emplace_back(TextTableCompuScale(ttcs));
            }

            if (const auto& cdv = node.child("COMPU-INTERNAL-TO-PHYS").child("COMPU-DEFAULT-VALUE")) {
                m_compuDefaultValue = CompuConst{ cdv.child_value("VT"), cdv.child("VT").attribute("TI").value() };
            }
        }

        if (m_category == "LINEAR") {
            for (const auto& lcs : node.child("COMPU-INTERNAL-TO-PHYS").child("COMPU-SCALES").children()) {
                m_linearCompuScales.emplace_back(LinearCompuScale(lcs));
            }

            if (const auto& cdv = node.child("COMPU-INTERNAL-TO-PHYS").child("COMPU-DEFAULT-VALUE")) {
                m_compuDefaultValue = cdv.child("VT").text().as_double();
            }
        }
    }


    DataObjectProp::CompuMethod::TextTableCompuScale::TextTableCompuScale(const pugi::xml_node& node)
    {
        m_compuConst = CompuConst{ node.child("COMPU-CONST").child_value("VT"),
                                   node.child("COMPU-CONST").child("VT").attribute("TI").value()
        };
        if (const auto& sl = node.child("SHORT-LABEL")) {
            m_shortLabel = sl.value();
        }
        if (const auto& ll = node.child("LOWER-LIMIT")) {
            m_lowerLimit = ll.text().as_uint();
        }
        if (const auto& ul = node.child("UPPER-LIMIT")) {
            m_upperLimit = ul.text().as_uint();
        }
    }

    DataObjectProp::CompuMethod::LinearCompuScale::LinearCompuScale(const pugi::xml_node& node)
    {
        m_description = ""; //getDescriptionFromXml(node);
        const auto& v = node.child("COMPU-RATIONAL-COEFFS").child("COMPU-NUMERATOR").child("V");
        m_compuNumerators[0] = v.text().as_double();
        m_compuNumerators[1] = v.next_sibling().text().as_double();
        m_compuDenominator = node.child("COMPU-RATIONAL-COEFFS").child("COMPU-DENOMINATOR").child("V").text().as_double();
    }

    DataObjectProp::InternalConstr::InternalConstr(const pugi::xml_node& node)
    {
        m_lowerLimit = node.child("LOWER-LIMIT").text().as_uint();
        m_upperLimit = node.child("UPPER-LIMIT").text().as_uint();

        // m_lowerLimitType = node.child("LOWER-LIMIT").attribute("INTERVAL-TYPE").value();
        // m_lowerLimitType = node.child("LOWER-LIMIT").attribute("INTERVAL-TYPE").value();

        for (auto& sc : node.child("SCALE-CONSTRS").children("SCALE-CONSTR")) {
            m_scaleConstrs.emplace_back(ScaleConstr(sc));
        }
    }
    DataObjectProp::InternalConstr::ScaleConstr::ScaleConstr(const pugi::xml_node& node)
    {
        m_validity = node.attribute("VALIDITY").value();
        m_shortLabel = node.child_value("SHORT-LABEL");
        m_shortLabelTI = node.child("SHORT-LABEL").attribute("TI").value();
        m_lowerLimit = node.child("LOWER-LIMIT").text().as_uint();
        m_upperLimit = node.child("UPPER-LIMIT").text().as_uint();
    }
    // ----- End: DataObjectProp -----

    // ----- Start: DTC -----
    DTC::DTC(const pugi::xml_node& node)
        : BasicInfo(node)
    {
        m_displayTroubleCode = node.child_value("DISPLAY-TROUBLE-CODE");
        m_text = node.child_value("TEXT");
        m_level = node.child("LEVEL").text().as_uint();
        m_troubleCode = node.child("TROUBLE-CODE").text().as_uint();
    }
    // ----- End: DTC -----

    // ----- Start: DtcDop -----
    DtcDop::DtcDop(const pugi::xml_node& node)
        : DataObjectProp(node)
    {
        m_isVisible = node.attribute("IS-VISIBLE").as_bool();
        // dtcs
        for (auto& dtc : node.child("DTCS").children("DTC")) {
            auto id = getIdFromXml(dtc);
            m_dtcs[id] = CreateRef<DTC>(dtc);
        }

        for (auto& dtc : node.child("DTCS").children("DTC-REF")) {
            auto id = getIdRefFromXml(dtc);
            auto doc = getDocRefFromXml(dtc);
            m_dtcs[id] = PDX::get().getDtcByDocAndId(doc, id);
        }
    }
    // ----- End: DtcDop -----

    // ----- Start: Param -----
    Param::Param(const pugi::xml_node& node)
    {
        m_shortName = node.child_value("SHORT-NAME");
        m_longName = node.child_value("LONG-NAME");
        for (const auto& p : node.child("DESC").children()) {
            m_description += p.first_child().value();
        }

        m_type = node.attribute("xsi:type").value();
        m_semantic = node.attribute("SEMANTIC").value();
        m_bytePosition = node.child("BYTE-POSITION").text().as_uint();
        m_bitPosition = node.child("BIT-POSITION").text().as_uint();
    }

    CodedConstParam::CodedConstParam(const pugi::xml_node& node)
        : Param(node)
    {
        m_diagCodedType = DiagCodedType(node.child("DIAG-CODED-TYPE"));
        m_codedValue = node.child("CODED-VALUE").text().as_uint();
    }

    ParamWithDop::ParamWithDop(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : Param(node)
    {
        if (const auto& dopRef = node.child("DOP-REF")) {
            String id = getIdRefFromXml(dopRef);
            String doc = getDocRefFromXml(dopRef);
            m_dopRef = Reference{ id, doc };
        }
        if (const auto& dopSNRef = node.child("DOP-SNREF")) {
            m_dopSNRef = dopSNRef.attribute("SHORT-NAME").as_string();
        }
    }

    void ParamWithDop::dereference(DiagLayerContainer* parentDlc)
    {
        if (m_dopSNRef.has_value()) {
            if (m_dopSNRef->substr(0, 3) == "DOP") {
                m_dop = parentDlc->m_dataObjectProps.at(*m_dopSNRef);
            }
        }
        else if (m_dopRef.has_value()) {
            auto id = m_dopRef->idRef;
            auto doc = m_dopRef->docRef;
            m_dop = PDX::get().getDopByDocAndId(doc, id);
        }
    }

    ValueParam::ValueParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : ParamWithDop(node, parentDlc)
    {
        m_physicalDefaultValue = node.child_value("PHYSICAL-DEFAULT-VALUE");
    }

    PhysConstParam::PhysConstParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : ParamWithDop(node, parentDlc)
    {
        m_physConstantValue = node.child_value("PHYS-CONSTANT-VALUE");
    }

    ReservedParam::ReservedParam(const pugi::xml_node& node)
        : Param(node)
    {
        m_bitLength = node.child("BIT-LENGTH").text().as_uint();
        m_diagCodedType = DiagCodedType(node.child("DIAG-CODED-TYPE"));
    }

    TableKeyParam::TableKeyParam(const pugi::xml_node& node)
        : Param(node)
    {
        m_id = getIdFromXml(node);
        m_tableRef = Reference(node.child("TABLE-REF"));

    }

    void TableKeyParam::dereference(const Map<String, Ref<Table>>& tableMap) {
        const auto& id = m_tableRef.idRef;
        if (tableMap.count(id)) {
            m_table = tableMap.at(id);
        }
        else {
            const auto& doc = m_tableRef.docRef;
            m_table = PDX::get().getTableByDocAndId(doc, id);
        }
        if (!m_table) {
            WH_ERROR("No Table found!");
        }
    }

    TableStructParam::TableStructParam(const pugi::xml_node& node)
        : Param(node)
    {
        m_tableKeyRef = Reference(node.child("TABLE-KEY-REF"));
    }

    void TableStructParam::dereference(Ref<StructBase> parentStuct)
    {
        String idRef = m_tableKeyRef.idRef;
        for (Ref<Param> param : parentStuct->m_params) {
            if (param->m_type == "TABLE-KEY") {
                m_tableKey = std::static_pointer_cast<TableKeyParam>(param);
            }
        }
        if (!m_tableKey || m_tableKey->m_id != idRef) {
            WH_ERROR("No TableKeyParam found!");
        }
    }

    MatchingRequestParam::MatchingRequestParam(const pugi::xml_node& node)
        : Param(node)
    {
        m_requestBytePos = node.child("REQUEST-BYTE-POS").text().as_uint();
        m_byteLength = node.child("BYTE-LENGTH").text().as_uint();
    }

    LengthKeyParam::LengthKeyParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : ParamWithDop(node, parentDlc)
    {
        m_id = getIdFromXml(node);
    }
    // ----- End: Param -----

    StructBase::StructBase(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : DopBase(node)
    {
        // params
        for (const auto& param : node.child("PARAMS").children()) {
            auto type = getXsiTypeFromXml(param);
            if (type == "VALUE") {
                m_params.emplace_back(CreateRef<ValueParam>(ValueParam(param, parentDlc)));
            }
            else if (type == "RESERVED") {
                m_params.emplace_back(CreateRef<ReservedParam>(ReservedParam(param)));
            }
            else if (type == "CODED-CONST") {
                m_params.emplace_back(CreateRef<CodedConstParam>(CodedConstParam(param)));
            }
            else if (type == "PHYS-CONST") {
                m_params.emplace_back(CreateRef<PhysConstParam>(PhysConstParam(param, parentDlc)));
            }
            else if (type == "LENGTH-KEY") {
                m_params.emplace_back(CreateRef<LengthKeyParam>(LengthKeyParam(param, parentDlc)));
            }
            else if (type == "MATCHING-REQUEST-PARAM") {
                m_params.emplace_back(CreateRef<MatchingRequestParam>(MatchingRequestParam(param)));
            }
            else if (type == "TABLE-KEY") {
                m_params.emplace_back(CreateRef<TableKeyParam>(TableKeyParam(param)));
            }
            else if (type == "TABLE-STRUCT") {
                m_params.emplace_back(CreateRef<TableStructParam>(TableStructParam(param)));
            }
        }
    }

    // ----- Start: Structure -----
    Structure::Structure(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : StructBase(node, parentDlc)
    {
        m_byteSize = node.child("BYTE-SIZE").text().as_uint();
    }
    // ----- End: Structure -----

    // ----- Start: Request -----
    Request::Request(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : StructBase(node, parentDlc)
    {
    }
    // ----- End: Request -----

    // ----- Start: Response -----
    Response::Response(const pugi::xml_node& node, DiagLayerContainer* parentDlc)
        : StructBase(node, parentDlc)
    {
    }
    // ----- End: Response -----

    // ----- Start: StaticField -----
    StaticField::StaticField(const pugi::xml_node& node, const Map<String, Ref<Structure>>& structMap)
        : BasicInfo(node)
    {
        auto id = getIdRefFromXml(node.child("BASIC-STRUCTURE-REF"));
        if (structMap.count(id)) {
            m_basicStructure = structMap.at(id);
        }
        else {
            auto doc = getDocRefFromXml(node.child("BASIC-STRUCTURE-REF"));
            m_basicStructure = PDX::get().getStructureByDocAndId(doc, id);
            if (!m_basicStructure) {
                WH_ERROR("No structure found!");
            }
        }

        m_fixedNumberOfItems = node.child("FIXED-NUMBER-OF-ITEMS").text().as_uint();
        m_itemByteSize = node.child("ITEM-BYTE-SIZE").text().as_uint();
    }
    // ----- End: StaticField -----

    // ----- Start: DynamicLengthField -----
    DynamicLengthField::DynamicLengthField(
        const pugi::xml_node& node,
        const Map<String, Ref<Structure>>& structMap,
        const Map<String, Ref<DataObjectProp>>& dopMap
    )
        : BasicInfo(node)
    {
        m_isVisible = node.attribute("IS-VISIBLE").as_bool();;
        m_offset = node.child("OFFSET").text().as_uint();
        m_basicStructureRef = Reference(node.child("BASIC-STRUCTURE-REF"));

        auto id = m_basicStructureRef.idRef;
        if (structMap.count(id)) {
            m_basicStructure = structMap.at(id);
        }
        else {
            auto doc = m_basicStructureRef.docRef;
            m_basicStructure = PDX::get().getStructureByDocAndId(doc, id);
            if (!m_basicStructure) {
                WH_ERROR("No structure found!");
            }
        }

        m_determinNumberOfItems = DeterminNumberOfItems(node.child("DETERMINE-NUMBER-OF-ITEMS"), dopMap);
    }

    DynamicLengthField::DeterminNumberOfItems::DeterminNumberOfItems(
        const pugi::xml_node& node,
        const Map<String, Ref<DataObjectProp>>& dopMap
    )
    {
        m_bytePosition = node.child("BYTE-POSITION").text().as_uint();
        m_bitPosition = node.child("BIT-POSITION").text().as_uint();
        m_dataObjectPropRef = Reference(node.child("DATA-OBJECT-PROP-REF"));

        Ref<DataObjectProp> m_dataObjectProp;

        auto id = m_dataObjectPropRef.idRef;
        if (dopMap.count(id)) {
            m_dataObjectProp = dopMap.at(id);
        }
        else {
            auto doc = m_dataObjectPropRef.docRef;
            m_dataObjectProp = PDX::get().getDataObjectPropByDocAndId(doc, id);
        }
        if (!m_dataObjectProp) {
            WH_ERROR("No data object prop found!");
        }
    }
    // ----- End: DynamicLengthField -----

    // ----- Start: EndOfPduField -----
    EndOfPduField::EndOfPduField(const pugi::xml_node& node, const Map<String, Ref<Structure>>& structMap)
        : BasicInfo(node)
    {
        auto id = getIdRefFromXml(node.child("BASIC-STRUCTURE-REF"));
        if (structMap.count(id)) {
            m_basicStruct = structMap.at(id);
        }
        else {
            auto doc = getDocRefFromXml(node.child("BASIC-STRUCTURE-REF"));
            m_basicStruct = PDX::get().getStructureByDocAndId(doc, id);
            if (!m_basicStruct) {
                WH_ERROR("No structure found!");
            }
        }
        m_maxItems = node.child("MAX-NUMBER-OF-ITEMS").text().as_uint();
        m_minItems = node.child("MIN-NUMBER-OF-ITEMS").text().as_uint();

        m_isVisible = node.attribute("IS-VISIBLE").as_bool();
    }
    // ----- End: EndOfPduField -----

    // ----- Start: Mux -----
    Mux::SwitchKey::SwitchKey(const pugi::xml_node& node) {
        m_bytePosition = node.child("BYTE-POSITION").text().as_uint();
        m_bitPosition = node.child("BIT-POSITION").text().as_uint();
        m_dopRef = Reference(node.child("DATA-OBJECT-PROP-REF"));
    }

    Mux::MuxCase::MuxCase(const pugi::xml_node& node) {
        m_shortName = node.child_value("SHORT-NAME");
        m_longName = node.child_value("LONG-NAME");
        m_strutureRef = Reference(node.child("STRUCTURE-REF"));
        m_lowerLimit = node.child("LOWER-LIMIT").text().as_uint();
        m_upperLimit = node.child("UPPER-LIMIT").text().as_uint();
    }

    Mux::Mux(const pugi::xml_node& node) : BasicInfo(node)
    {
        m_isVisible = node.attribute("IS-VISIBLE").as_bool();
        m_bytePosition = node.child("BYTE-POSITION").text().as_uint();
        m_switchKey = SwitchKey(node.child("SWITCH-KEY"));
        m_defaultCase = MuxCase(node.child("DEFAULT-CASE"));

        for (pugi::xml_node _case : node.child("CASES").children()) {
            m_cases.push_back(MuxCase(_case));
        }
    }
    // ----- End: Mux -----

    // ----- Start: Table -----
    Table::TableRow::TableRow(const pugi::xml_node& node, const Map<String, Ref<Structure>>& structMap)
        : BasicInfo(node)
    {
        const auto& id = getIdRefFromXml(node.child("STRUCTURE-REF"));
        if (structMap.count(id)) {
            m_structure = structMap.at(id);
        }
        else {
            const auto& doc = getDocRefFromXml(node.child("STRUCTURE-REF"));
            m_structure = PDX::get().getStructureByDocAndId(doc, id);
            if (!m_structure) {
                WH_ERROR("No structure found!");
            }
        }
        m_key = node.child_value("KEY");
    }

    Table::Table(const pugi::xml_node& node,
        const Map<String, Ref<DataObjectProp>>& dopMap,
        const Map<String, Ref<Structure>>& structMap)
        : BasicInfo(node)
    {
        m_semantic = node.attribute("SEMANTIC").value();

        const auto& id = getIdRefFromXml(node.child("KEY-DOP-REF"));
        if (dopMap.count(id)) {
            m_keyDop = dopMap.at(id);
        }
        else {
            const auto& doc = getDocRefFromXml(node.child("KEY-DOP-REF"));
            m_keyDop = PDX::get().getDataObjectPropByDocAndId(doc, id);
        }

        if (!m_keyDop) {
            WH_ERROR("No Key DOP found!");
        }

        for (auto& child : node.children()) {
            if (!strcmp(child.name(), "TABLE-ROW")) {
                m_tableRows.emplace_back(CreateRef<TableRow>(TableRow(child, structMap)));
            }
            else if (!strcmp(child.name(), "TABLE-ROW-REF")) {
                m_tableRows.emplace_back(Reference(child));
            }
        }
    }
    // ----- End: Table -----



    // ----- Start: FunctClass -----

    FunctClass::FunctClass(const pugi::xml_node& node)
        : BasicInfo(node)
    {
    }
    // ----- End: FunctClass -----

    // ----- Start: DiagService -----
    DiagService::DiagService(const pugi::xml_node& node)
        : BasicInfo(node)
    {

        m_addressing = node.attribute("ADDRESSING").value();
        m_semantic = node.attribute("SEMANTIC").value();
        /*for (const auto& fc : node.child("FUNCT-CLASS-REFS").children("FUNCT-CLASS-REF")) {
            m_functClassRefs.emplace_back(Reference(fc));
        }*/
        m_requestRef = Reference(node.child("REQUEST-REF"));
        m_posResponseRef = Reference(node.child("REQUEST-REF"));
        m_negResponseRef = Reference(node.child("REQUEST-REF"));
    }
    std::optional<std::string> DiagService::decode(const std::string& message)
    {
        return std::optional<std::string>();
    }
    // ----- End: Class DiagService -----

    // ----- Start: SingleEcuJob -----
    SingleEcuJob::SingleEcuJob(const pugi::xml_node& node)
        : BasicInfo(node)
    {

        // funct class refs
        /*for (auto& fc : node.child("FUNCT-CLASS-REFS").children("FUNCT-CLASS-REF")) {
            m_functClassRefs.emplace_back(Reference(fc));
        }

        m_audience = Audience{
            node.attribute("IS-SUPPLIER").as_bool(),
            node.attribute("IS-DEVELOPMENT").as_bool(),
            node.attribute("IS-MANUFACTURING").as_bool(),
            node.attribute("IS-AFTERSALES").as_bool()
        };*/

        // prog codes
        for (auto& pc : node.child("PROG-CODES").children("PROG-CODE")) {
            m_progCodes.emplace_back(
                ProgCode{ pc.child_value("CODE-FILE"),
                          pc.child_value("SYNTAX"),
                          pc.child_value("REVISION"),
                          pc.child_value("ENTRYPOINT")
                }
            );
        }

        // input params
        for (auto& ip : node.child("INPUT-PARAMS").children("INPUT-PARAM")) {
            m_inputParams.emplace_back(
                InputParam{ ip.child_value("SHORT-NAME"),
                            ip.child_value("LONG-NAME"),
                            ip.child_value("PHYSICAL-DEFAULT-VALUE"),
                            Reference(ip.child("DOP-BASE-REF"))
                }
            );
        }
    }
    // ----- End: SingleEcuJob -----

    // ----- Start: EcuVariantPattern -----
    EcuVariantPattern::EcuVariantPattern(const pugi::xml_node& node)
    {
        for (auto& mp : node.child("MATCHING-PARAMETERS").children()) {
            m_matchingParameters.emplace_back(
                MatchingParameter{
                    mp.child_value("EXPECTED-VALUE"),
                    mp.child("DIAG-COMM-SNREF").attribute("SHORT-NAME").value(),
                    mp.child("OUT-PARAM-IF-SNREF").attribute("SHORT-NAME").value()
                }
            );
        }
    }
    // ----- End: SinglEcuVariantPatterneEcuJob -----

    // ----- Start: Class ParentRef -----
    ParentRef::ParentRef(const pugi::xml_node& node, const Ref<DiagLayerContainer>& dlc)
        : Reference(node), xsiType(getXsiTypeFromXml(node)), parentDlc(dlc)
    {
        for (auto& diagComm : node.child("NOT-INHERITED-DIAG-COMMS").children()) {
            notInheritedDiagComms.insert(diagComm.child("DIAG-COMM-SNREF").attribute("SHORT-NAME").value());
        }
        parentRefType = ParentRefType[xsiType];
    }
    // ----- End: Class ParentRef -----

    // ---- Start: DiagLayerContainer -----
    DiagLayerContainer::DiagLayerContainer(const String& dlcName)
    {
        WH_INFO("Initializing DLC [{}]", dlcName);
        pugi::xml_document doc;
        auto fileName = PDX::get().getDlcFileNameById(dlcName);
        pugi::xml_parse_result result = doc.load_file(fileName.c_str());

        if (!result) {
            WH_ERROR("Not a valid Diag-Layer-Container Node.");
        }

        pugi::xml_node node = doc.find_node([dlcName](pugi::xml_node subnode) {
            return dlcName == subnode.attribute("ID").as_string();
            });

        // info
        m_containerType = node.name();
        m_id = node.attribute("ID").value();
        m_shortName = node.child_value("SHORT-NAME");
        m_longName = node.child_value("LONG-NAME");
        //m_description = getDescriptionFromXml(node);

        // Import Refs
        for (const auto& ref : node.child("IMPORT-REFS").children()) {
            auto id = getIdRefFromXml(ref);
            if (!PDX::get().getDlcById(id)) {
                m_referencedDlcs[id] = PDX::get().addDlcById(id);
            }
            // m_importRefs.emplace_back(Reference(ref));
        }

        // Parent Refs
        for (const auto& ref : node.child("PARENT-REFS").children()) {
            auto parentId = getIdRefFromXml(ref);
            auto parentDlc = PDX::get().getDlcById(parentId);
            if (!parentDlc) {
                parentDlc = PDX::get().addDlcById(parentId);
            }
            m_parentRefs[parentId] = ParentRef(ref, parentDlc);

            /*bool alreadyParented = false;
            for (auto& [id, grandParent] : parentDlc->m_parentRefs) {
                if (m_parentRefs.count(id)) {
                    m_parentRefs.at(id).notInheritedDiagComms.merge(grandParent.notInheritedDiagComms);
                }
                else {
                    m_parentRefs[id] = grandParent;
                }
            }*/
        }

        // Diag Data Dictionary Spec
        auto specNode = node.child("DIAG-DATA-DICTIONARY-SPEC");


        // PhysicalDimension and Unit 
        if (const auto& unitSpec = specNode.child("UNIT-SPEC")) {
            // PhysicalDimension
            for (auto& phd : unitSpec.child("PHYSICAL-DIMENSIONS").children("PHYSICAL-DIMENSION")) {
                String id = phd.attribute("ID").value();
                m_physicalDimensions[id] = CreateRef<PhysicalDimension>(PhysicalDimension(phd));
            }

            for (auto& unit : unitSpec.child("UNITS").children("UNIT")) {
                String id = unit.attribute("ID").value();
                m_units[id] = CreateRef<Unit>(Unit(unit, m_physicalDimensions));
            }
        }

        // dtc dops
        for (const auto& dd : specNode.child("DTC-DOPS").children()) {
            String id = getIdFromXml(dd);
            m_dtcDops[id] = CreateRef<DtcDop>(DtcDop(dd));
        }

        // dataObjectProps
        for (auto& dataObjProp : specNode.child("DATA-OBJECT-PROPS").children()) {
            String id = getIdFromXml(dataObjProp);
            m_dataObjectProps[id] = CreateRef<DataObjectProp>(DataObjectProp(dataObjProp));
        }

        // structures
        for (auto& structure : specNode.child("STRUCTURES").children()) {
            String id = getIdFromXml(structure);
            m_structures[id] = CreateRef<Structure>(Structure(structure, this));
        }

        // staticFields
        for (auto& staticField : specNode.child("STATIC-FIELDS").children()) {
            String id = getIdFromXml(staticField);
            m_staticFields[id] = CreateRef<StaticField>(StaticField(staticField, m_structures));
        }

        // Dynamic Length Fields
        for (auto& dlf : specNode.child("DYNAMIC-LENGTH-FIELDS").children()) {
            String id = getIdFromXml(dlf);
            m_dynamicLengthFields[id] = CreateRef<DynamicLengthField>(DynamicLengthField(dlf, m_structures, m_dataObjectProps));
        }

        // End of PDU Fields
        for (auto& eopf : specNode.child("END-OF-PDU-FIELDS").children()) {
            String id = getIdFromXml(eopf);
            m_endOfPduFields[id] = CreateRef<EndOfPduField>(EndOfPduField(eopf, m_structures));
        }

        // Muxes
        for (auto& mux : specNode.child("MUXS").children()) {
            String id = getIdFromXml(mux);
            m_muxes[id] = CreateRef<Mux>(Mux(mux));
        }

        // Tables
        for (auto& table : specNode.child("TABLES").children()) {
            String id = getIdFromXml(table);
            m_tables[id] = CreateRef<Table>(Table(table, m_dataObjectProps, m_structures));
        }


        // Funct Class
        /*for (auto& fc : node.child("FUNCT-CLASSS").children()) {
            String id = getIdFromXml(fc);
            this->m_funcClasses[id] = CreateRef<FunctClass>(FunctClass(fc));
        }*/

        // ----- DiagComms -----
        for (auto& diagComm : node.child("DIAG-COMMS").children()) {
            if (!strcmp(diagComm.name(), "DIAG-SERVICE")) {
                String id = getIdFromXml(diagComm);
                String shortName = getShortNameFromXml(diagComm);
                auto dct = DiagCommType::DiagService;
                m_diagComms[id] = DiagComm{
                    shortName,
                    dct,
                    CreateRef<DiagService>(DiagService(diagComm)),
                    nullptr
                };
            }
            else if (!strcmp(diagComm.name(), "SINGLE-ECU-JOB")) {
                String id = getIdFromXml(diagComm);
                String shortName = getShortNameFromXml(diagComm);
                auto dct = DiagCommType::SingleEcuJob;
                m_diagComms[id] = DiagComm{
                    shortName,
                    dct,
                    nullptr,
                    CreateRef<SingleEcuJob>(SingleEcuJob(diagComm)),
                };
            }
            else if (!strcmp(diagComm.name(), "DIAG-COMM-REF")) {
                auto id = getIdRefFromXml(diagComm);
                auto doc = getDocRefFromXml(diagComm);

                m_diagComms[id] = PDX::get().getDiagCommByDocAndId(doc, id);
            }
        }

        // Requests
        for (auto& request : node.child("REQUESTS").children("REQUEST")) {
            String id = getIdFromXml(request);
            m_requests[id] = CreateRef<Request>(Request(request, this));
        }

        // Pos Responses
        for (auto& pr : node.child("POS-RESPONSES").children("POS-RESPONSE")) {
            String id = getIdFromXml(pr);
            m_posResponses[id] = CreateRef<Response>(Response(pr, this));
        }

        // Neg Responses
        for (auto& nr : node.child("NEG-RESPONSES").children("NEG-RESPONSE")) {
            String id = getIdFromXml(nr);
            m_negResponses[id] = CreateRef<Response>(Response(nr, this));
        }
        for (auto& gnr : node.child("GLOBAL-NEG-RESPONSES").children()) {
            String id = getIdFromXml(gnr);
            m_globalNegResponses[id] = CreateRef<Response>(Response(gnr, this));
        }

        // ComParam Refs
        /*for (auto& cpr : node.child("COMPARAM-REFS").children("COMPARAM-REF")) {
            m_comparamRefs.emplace_back(ComParamRef(cpr));
        }*/

        // Ecu Variant Patterns
        /*for (auto& pattern : node.child("ECU-VARIANT-PATTERNS").children("ECU-VARIANT-PATTERN")) {
            m_ecuVariantPatterns.emplace_back(EcuVariantPattern(pattern));
        }*/

        /*std::cout << "Processing inheritance for: [" << m_id << "]\nsize: " << m_diagComms.size();
        for (auto parentRef : m_parentRefs) {
            auto parentDlc = parentRef.parentDlc;
            auto parentDiagComms = parentDlc->getAllDiagComms();
            if (parentDiagComms.size() > 0) {
                for (auto& [id, value] : parentDiagComms) {
                    if (parentRef.notInheritedDiagComms.count(id)) {
                        std::cout << "\t\tDiagComm [" << id << "] is not inherited.\n";
                    }
                    else {
                        if (m_diagComms.count(id)) {
                            std::cout << "\t\tDiagComm [" << id << "] exist, ignored!\n";
                        }
                        else {
                            std::cout << "\t\tDiagComm [" << id << "] inherited\n";
                        }
                        m_diagComms[id] = value;
                    }
                }
            }
        }
        std::cout << " After inheritance: " << m_diagComms.size() << "\n";*/
        /*auto
            if (parentDiagComms.size() > 0) {
                for (auto& [id, value] : parentDiagComms) {
                    if (!parentRef.notInheritedDiagComms.count(id)) {
                        if (m_diagComms.count(id)) {
                            std::cout << "\t\tDiagComm [" << id << "] override" << std::endl;
                        }
                        else {
                            std::cout << "\t\tDiagComm [" << id << "] inherited" << std::endl;
                        }
                        m_diagComms[id] = value;
                    }
                    else {
                        std::cout << "\t\tDiagComm [" << id << "] is not inherited." << std::endl;
                    }
                }
            }*/

        /*if (m_containerType == "ECU-VARIANT") {
            m_parentRefs.begin()->second.parentDlc->m_subEcuVariants.push_back(m_id);
        }*/
    }


    Ref<FunctClass> DiagLayerContainer::getFunctClassById(const String& id) const
    {
        /*if (m_funcClasses.count(id)) {
            return m_funcClasses.at(id);
        }*/
        return nullptr;
    }

    Ref<Request> DiagLayerContainer::getRequestById(const String& id) const
    {
        return m_requests.at(id);
    }

    Ref<DopBase> DiagLayerContainer::getDopById(const String& id) const
    {
        auto dopType = id.substr(0, 3);

        if (dopType == "DOP") {
            return getDataObjectPropById(id);
        }
        if (dopType == "DTC") {
            return getDtcDopById(id);
        }
        else if (dopType == "MUX") {
            //return getMuxByid(id);
        }
        else if (dopType == "STR") {
            //return getStructureById(id);
        }
        else if (dopType == "EOP") {
            //return getEndOfPduFieldById(id);
        }
        return nullptr;
    }

    Ref<DataObjectProp> DiagLayerContainer::getDataObjectPropById(const String& id) const
    {
        if (m_dataObjectProps.count(id)) {
            return m_dataObjectProps.at(id);
        }
        return nullptr;
    }

    Ref<DtcDop> DiagLayerContainer::getDtcDopById(const String& id) const
    {
        if (m_dtcDops.count(id)) {
            return m_dtcDops.at(id);
        }
        return nullptr;
    }

    Ref<Structure> DiagLayerContainer::getStructureById(const String& id) const
    {
        if (m_structures.count(id)) {
            return m_structures.at(id);
        }
        return nullptr;
    }

    Ref<Table> DiagLayerContainer::getTableById(const String& id) const
    {
        if (m_tables.count(id)) {
            return m_tables.at(id);
        }
        return nullptr;
    }

    Ref<Unit> DiagLayerContainer::getUnitById(const String& id) const
    {
        if (m_units.count(id)) {
            return m_units.at(id);
        }
        return nullptr;
    }

    Ref<DTC> DiagLayerContainer::getDtcById(const String& id) const
    {
        auto pos = id.find('.');
        auto dtcDopName = id.substr(0, pos);

        if (m_dtcDops.count(dtcDopName)) {
            auto& dtcDop = m_dtcDops.at(dtcDopName);
            if (dtcDop->m_dtcs.count(id)) {
                return dtcDop->m_dtcs.at(id);
            }
        }
        return nullptr;
    }

    DiagComm DiagLayerContainer::getDiagCommById(const String& id) const
    {
        if (m_diagComms.count(id)) {
            return m_diagComms.at(id);
        }
        WH_ERROR("DiagComm [" + id + "] not found in dlc [" + m_id + "].");
    }

    Ref<PhysicalDimension> DiagLayerContainer::getPhysicalDimensionById(const String& id) const
    {
        if (m_physicalDimensions.count(id)) {
            return m_physicalDimensions.at(id);
        }
        return nullptr;
    }

    //void DiagLayerContainer::loadParent() {
    //	for (auto parentRef : m_parentRefs) {
    //		auto parentDiagCommss = parentRef.parentDlc->getAllDiagComms();
    //		for (auto& [id, diagService] : parentDiagServices) {
    //			if (!parentRef.notInheritedDiagComms.count(id)) {
    //				m_diagServices[id] = diagService;
    //			}
    //		}
    //	}
    //	m_parentLoaded = true;
    //}

    const Map<String, DiagComm>& DiagLayerContainer::getAllDiagComms() const
    {
        return m_diagComms;
    }

    void DiagLayerContainer::inherit() {
        for (const auto& type : { "PROTOCOL-REF", "FUNCTIONAL-GROUP-REF", "BASE-VARIANT-REF", "ECU-SHARED-DATA-REF" }) {
            for (auto& [id, parentRef] : m_parentRefs) {
                std::cout << "Current parent: " + id << std::endl;
                if (parentRef.xsiType == type) {
                    std::cout << "\tGettring inherited services from: [" << id << "]\n";
                    auto parentDiagComms = parentRef.parentDlc->getAllDiagComms();
                    if (parentDiagComms.size() > 0) {
                        for (auto& [id, value] : parentDiagComms) {
                            auto shortName = value.shortName;
                            if (parentRef.notInheritedDiagComms.count(shortName)) {
                                std::cout << "\tDiagComm [" << id << "] is not inherited." << std::endl;
                            }
                            else {
                                if (m_diagComms.count(id)) {
                                    std::cout << "\tDiagComm [" << id << "] exist, ignored!" << std::endl;
                                }
                                else {
                                    std::cout << "\tDiagComm [" << id << "] inherited from [" + parentRef.docRef + "]" << std::endl;
                                    m_diagComms[id] = value;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Vec<Ref<DiagService>> DiagLayerContainer::getAllDiagServices() {

        auto allDiagComms = getAllDiagComms();
        Vec<Ref<DiagService>> allServices;
        for (auto [id, diagComm] : allDiagComms) {
            if (diagComm.type == DiagCommType::DiagService) {
                allServices.push_back(diagComm.diagService);
            }
        }
        return allServices;
    }

    /*const Vec<String>& DiagLayerContainer::getSubEvShortNames(const String&) const
    {
        WH_INFO("In ev: {}", m_id);
        for (auto subev : m_subEcuVariants) {
            WH_INFO("\tcurrent sub ev: {}", subev);
        }
        return m_subEcuVariants;
    }*/

    std::set<std::shared_ptr<DiagService>> DiagLayerContainer::getDiagServicesByValue(unsigned value) const {
        std::set<std::shared_ptr<DiagService>> allDiagComms;
        /*for (auto& [key, diagComm] : m_diagServices) {
            if (diagComm->m_request->m_params.at(0).m_codedValue == value) {
                allDiagComms.insert(diagComm);
            }
        }*/
        /*for (auto& parentDlc : m_parentDlcs) {
                allDiagComms.merge(parentDlc->getDiagCommsByValue(value));
        }*/
        return allDiagComms;
    }
    // ----- End: DiagLayerContainer -----

    // ----- Start: Class VehicleInformation -----
    VehicleInformation::VehicleInformation(const pugi::xml_node& root)
        : BasicInfo(root)
    {
        // logical links
        for (auto& node : root.child("LOGICAL-LINKS").children()) {
            m_logicalLinks.emplace_back(LogicalLink(node));
        }
    }

    String VehicleInformation::shortName() const
    {
        return m_shortName;
    }

    Vec<String> VehicleInformation::getLogicalLinkShortNames()
    {
        Vec<String> shortNames;
        for (auto& logicalLink : m_logicalLinks) {
            shortNames.push_back(logicalLink.shortName());
        }
        return shortNames;
    }

    Vec<String> VehicleInformation::getBvShortNames()
    {
        Vec<String> shortNames;
        for (auto& logicalLink : m_logicalLinks) {
            auto shortName = logicalLink.getBaseVariant();
            if (shortName.has_value()) {
                shortNames.push_back(*shortName);
            }
        }
        return shortNames;
    }

    // ----- End: Class VehicleInformation -----

    // ----- Start: Class LogicalLink -----
    LogicalLink::LogicalLink(const pugi::xml_node& node)
        : BasicInfo(node)
    {
        m_physicalVehicleLink = node.child("PHYSICAL-VEHICLE-LINK-REF").attribute("ID-REF").value();

        // protocol refs
        if (const auto& tempNode = node.child("PROTOCOL-REF")) {
            m_protocolRef = Reference(tempNode);
        }
        // functional group refs
        if (const auto& tempNode = node.child("FUNCTIONAL-GROUP-REF")) {
            m_functionalGroupRef = Reference(tempNode);
        }
        // base variant refs
        if (const auto& tempNode = node.child("BASE-VARIANT-REF")) {
            m_baseVariantRef = Reference(tempNode);
        }
        // link comparam refs
        if (const auto& tempNode = node.child("LINK-COMPARAM-REFS")) {
            for (pugi::xml_node node : tempNode.children()) {
                m_linkComParamRefs.emplace_back(Reference(node));
            }
        }
    }

    String LogicalLink::shortName() const
    {
        return m_shortName;
    }

    // ----- Start: Class UnitGroup -----
    UnitGroup::UnitGroup(const pugi::xml_node& node)
        : BasicInfo(node)
    {
        m_category = node.child_value("CATEGORY");
        // Unit Refs
        for (auto& ref : node.child("UNIT-REFS").children()) {
            m_unitRefs.emplace_back(Reference(ref));
        }
    }
    // ----- End: Class UnitGroup -----


    // ----- Start: Class PDX -----
    void PDX::init(std::filesystem::path path)
    {
        WH_INFO("Current path: {}", path);
        std::filesystem::current_path(path);
        
        pugi::xml_document doc; // vehicle info spec
        pugi::xml_parse_result result = doc.load_file("index.xml");

        Vec<String> evs;
        Vec<String> bvs;


        for (auto& ablock : doc.child("CATALOG").child("ABLOCKS").children()) {
            String fileName = ablock.child_value("SHORT-NAME");
            auto odxPos = fileName.find("odx");
            if (odxPos != std::string::npos) {
                fileName.replace(odxPos - 1, 1, ".");
                std::string fileType = fileName.substr(0, 2);
                if (fileType == "VI") {
                    m_visFile = fileName;
                }
                else if (fileType == "PR" || fileType == "FG" || fileType == "ES" || fileType == "BL") {
                    auto pos = fileName.find_first_of('_', 5);
                    m_dlcFiles[fileName.substr(0, pos)] = fileName;
                }
                else if (fileType == "BV") {
                    auto pos = fileName.find_first_of('_', 5);
                    auto shortName = fileName.substr(0, pos);
                    m_dlcFiles[shortName] = fileName;
                    bvs.push_back(shortName);
                }
                else if (fileType == "EV") {
                    auto pos = fileName.find("_d.odx");
                    auto shortName = fileName.substr(0, pos - 3);
                    m_dlcFiles[shortName] = fileName;
                    evs.push_back(shortName);
                }
                else if (fileType == "IS") {
                    cps.push_back(fileName);
                }
            }
        }

        for (auto& ev : evs) {
            if (doc.load_file(m_dlcFiles[ev].c_str())) {
                auto parent_bv = doc.select_node("/ODX/DIAG-LAYER-CONTAINER/ECU-VARIANTS/ECU-VARIANT/PARENT-REFS/PARENT-REF");
                if (parent_bv) {
                    String bv_id = parent_bv.node().attribute("ID-REF").value();
                    if (m_bvMapSubEvs.contains(bv_id)) {
                        m_bvMapSubEvs[bv_id].push_back(ev);
                    }
                    else {
                        m_bvMapSubEvs[bv_id] = { ev };
                    }
                    //WH_INFO("parent bv: {}", bv_id);
                }
            }
        }

        for (auto& bv : bvs) {
            if (doc.load_file(m_dlcFiles[bv].c_str())) {
                auto comparamRefs = doc.select_node("/ODX/DIAG-LAYER-CONTAINER/BASE-VARIANTS/BASE-VARIANT/COMPARAM-REFS");
                if (comparamRefs) {
                    //WH_INFO("parent bv: {}", bv);
                    for (const auto& cr : comparamRefs.node().children()) {
                        auto value = cr.child("VALUE").text().as_int();
                        if (value != 0 && !m_canIdMapBv.contains(value)) {
                            m_canIdMapBv[value] = bv;
                            //WH_INFO("insert can id and bv: {} - {}", value, bv);
                        }
                    }
                }
            }
        }

        auto bcm = getEvByShortName("EV_BCM37w_006");


        /*for (auto& [id, fileName] : m_dlcFiles) {
        	m_dlcMap[id] = CreateRef<DiagLayerContainer>(DiagLayerContainer(id));
        }*/

        pugi::xml_document visDoc;
        pugi::xml_parse_result visResult = visDoc.load_file(m_visFile.c_str());

        if (!visResult) {
            WH_ERROR("Can not open vehicle information file: {}", m_visFile);
        }
        pugi::xml_node vis = visDoc.child("ODX").first_child();

        // info components
        for (auto& node : vis.child("INFO-COMPONENTS").children()) {
            String xsiType = node.attribute("xsi:type").value();
            if (xsiType == "MODEL-YEAR") {
                m_modelYear = node.child("LONG-NAME").text().as_uint();
            }
            if (xsiType == "OEM") {
                m_oem = node.child_value("LONG-NAME");
            }
            if (xsiType == "VEHICLE-MODEL") {
                m_vehicleModel = node.child_value("LONG-NAME");
            }
        }

        // vehicle informations
        for (auto& node : vis.child("VEHICLE-INFORMATIONS").children()) {
            auto shortName = getShortNameFromXml(node);
            m_vehicleInfoMap[shortName] = CreateRef<VehicleInformation>(VehicleInformation(node));
        }


        for (auto& cp : cps) {
            auto pos = cp.find_last_of('_');
            m_comparamSpecs[cp.substr(0, pos)] = std::make_shared<ComParamSpec>(ComParamSpec(cp));
        }

        m_loaded = true;
    }

    Ref<DiagLayerContainer> PDX::getEvByShortName(const String& evName)
    {
        auto evDlc = getDlcById(evName);
        evDlc->inherit();
        return evDlc;
    }

    void PDX::initDLCByShortName(const String& dlcName)
    {
        if (!m_dlcMap.count(dlcName)) {

            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(m_dlcFiles[dlcName].c_str());

            if (result) {
                pugi::xml_node node = doc.child("ODX").child("DIAG-LAYER-CONTAINER").first_child();
                while (node &&
                    !(strcmp(node.name(), "ECU-SHARED-DATAS") == 0 ||
                      strcmp(node.name(), "BASE-VARIANTS") == 0 ||
                      strcmp(node.name(), "ECU-VARIANTS") == 0 ||
                      strcmp(node.name(), "FUNCTIONAL-GROUPS") == 0 ||
                      strcmp(node.name(), "PROTOCOLS") == 0)
                    ) {
                    node = node.next_sibling();
                }
                node = node.first_child();

                // Import Refs
                /*for (auto& ref : node.child("IMPORT-REFS").children()) {
                    initDLCByShortName(ref.attribute("ID-REF").value());
                }*/

                // auto dlc = new DiagLayerContainer(node, m_dlcMap);

                // m_dlcMap[dlcName] = std::make_shared<DiagLayerContainer>(DiagLayerContainer(node));
                std::cout << "Add DLC: " << dlcName << std::endl;
                //return dlc;
            }
        }
    }

    Ref<DiagLayerContainer> PDX::getDlcById(const String& id)
    {
        if (this->m_dlcMap.count(id)) {
            return this->m_dlcMap.at(id);
        }
        else {
            if (this->m_dlcFiles.count(id)) {
                return this->addDlcById(id);
            }
            else {
                WH_ERROR("Diag Layer Container [{}] not found!", id);
            }
        }
        return nullptr;
    }

    Ref<ComParamSpec> PDX::getComParamSpecById(const String& id)
    {
        if (m_comparamSpecs.count(id)) {
            return m_comparamSpecs.at(id);
        }
        return nullptr;
    }

    String PDX::getDlcFileNameById(const String& id)
    {
        String requestedFile = "";
        if (m_dlcFiles.count(id)) {
            requestedFile = m_dlcFiles.at(id);
        }
        return requestedFile;
    }

    Ref<DiagLayerContainer> PDX::addDlcById(const String& id)
    {
        if (m_dlcFiles.count(id)) {
            m_dlcMap[id] = CreateRef<DiagLayerContainer>(DiagLayerContainer(id));
        }
        return m_dlcMap[id];
    }

    Ref<DopBase> PDX::getDopByDocAndId(const String& doc, const String& id) {
        return m_dlcMap[doc]->getDopById(id);
    }

    Ref<Structure> PDX::getStructureByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getStructureById(id);
    }

    Ref<DataObjectProp> PDX::getDataObjectPropByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getDataObjectPropById(id);
    }

    Ref<Table> PDX::getTableByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getTableById(id);
    }

    Ref<Unit> PDX::getUnitByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getUnitById(id);
    }

    Ref<DTC> PDX::getDtcByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getDtcById(id);
    }

    Ref<PhysicalDimension> PDX::getPhysicalDimensionByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getPhysicalDimensionById(id);
    }

    DiagComm PDX::getDiagCommByDocAndId(const String& doc, const String& id) {
        return this->m_dlcMap[doc]->getDiagCommById(id);
    }

    // ----- END: Class PDX -----


    UnitSpec::UnitSpec(const pugi::xml_node& node)
    {
        // Unit Group
        for (pugi::xml_node ug : node.child("UNIT-GROUPS").children()) {
            String id = getIdFromXml(ug);
            m_unitGroups[id] = CreateRef<UnitGroup>(UnitGroup(ug));
        }

        // Unit
        for (auto& unit : node.child("UNIT-SPEC").child("UNITS").children("UNIT")) {
            String id = getIdFromXml(unit);
            m_units[id] = CreateRef<Unit>(Unit(unit, m_physicalDimensions));
        }

        // PhysicalDimension
        for (auto& phd : node.child("UNIT-SPEC").child("PHYSICAL-DIMENSIONS").children("PHYSICAL-DIMENSION")) {
            String id = getIdFromXml(phd);
            m_physicalDimensions[id] = CreateRef<PhysicalDimension>(PhysicalDimension(phd));
        }
    }

    ComParam::ComParam(const pugi::xml_node& node, const ComParamSpec* p_comparamSpec)
        : BasicInfo(node)
    {
        m_cpType = node.attribute("CPTYPE").value();
        m_displayLevel = node.attribute("DISPLAY-LEVEL").as_uint();
        m_paramClass = node.attribute("PARAM-CLASS").value();
        m_physicalDefaultValue = node.child_value("PHYSICAL-DEFAULT-VALUE");
        m_dop = p_comparamSpec->m_dataObjProps.at(node.child("DATA-OBJECT-PROP-REF").attribute("ID-REF").value());
    }

    ComParamSpec::ComParamSpec(const String& fileName)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(fileName.c_str());

        if (!result) {
            WH_ERROR("Failed to open file : " + fileName);
        }

        pugi::xml_node node = doc.child("ODX").first_child();

        // info
        m_id = getIdFromXml(node);
        m_shortName = node.child_value("SHORT-NAME");
        m_longName = node.child_value("LONG-NAME");


        // PhysicalDimension and Unit 
        if (const auto& unitSpec = node.child("UNIT-SPEC")) {
            // PhysicalDimension
            for (auto& phd : unitSpec.child("PHYSICAL-DIMENSIONS").children("PHYSICAL-DIMENSION")) {
                String id = getIdFromXml(phd);
                m_physicalDimensions[id] = CreateRef<PhysicalDimension>(PhysicalDimension(phd));
            }

            for (auto& unit : unitSpec.child("UNITS").children("UNIT")) {
                String id = getIdFromXml(unit);
                m_units[id] = CreateRef<Unit>(Unit(unit, m_physicalDimensions));
            }
        }

        // data object props
        for (auto& dop : node.child("DATA-OBJECT-PROPS").children()) {
            String id = getIdFromXml(dop);
            m_dataObjProps[id] = CreateRef<DataObjectProp>(DataObjectProp(dop));
        }

        // comparams
        for (auto& cp : node.child("COMPARAMS").children()) {
            String id = getIdFromXml(cp);
            m_comparams[id] = CreateRef<ComParam>(ComParam(cp, this));
        }
    }

    Ref<VehicleInformation> PDX::getVehicleInfoById(const String& id) {
        return m_vehicleInfoMap[id];
    }

    Vec<String> PDX::getVehicleInfoShortNames() const {
        Vec<String> shortNames;
        for (auto vis : m_vehicleInfoMap) {
            shortNames.emplace_back(vis.first);
        }

        return shortNames;
    }

    Vec<String> PDX::getLogicalLinksByVehicleInfoId(const String& id) {
        return m_vehicleInfoMap[id]->getLogicalLinkShortNames();
    }

    Vec<String> PDX::getBvShortNamesByVehicleInfoId(const String& id) {
        return m_vehicleInfoMap[id]->getBvShortNames();
    }

    Vec<String> PDX::getEvShortNamesByBvId(const String& id) {
        WH_INFO("request bv: {}", id);
        return m_bvMapSubEvs[id];
    }

}