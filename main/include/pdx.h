#pragma once

#include <fstream>
#include <pugixml.hpp>
#include <variant>
#include <set>
#include <optional>
#include <map>

#include "type.h"

#include "bus_log.h"


namespace whale {

    // ------------ Type Definitions ------------
    using DopSNRef = std::string;

    enum class DiagCommType {
        DiagService,
        SingleEcuJob
    };

    enum class ParentRefType {
        Protocol,
        FunctionalGroup,
        BasicVariant,
        EcuSharedData
    };
    // ------------------------------------------

    struct BasicInfo {
        String m_id;
        String m_shortName;
        String m_longName;
        //String m_description;

        BasicInfo() {}
        BasicInfo(const pugi::xml_node& node);
        String id() const;
        String shortName() const;
    };

    struct Reference {
        String idRef;
        String docRef;
        String docType;

        Reference() = default;
        Reference(const pugi::xml_node&);
        Reference(const Reference& rhs);
        Reference(String id, String doc = "", String type = "");
    };



    struct ComParamRef : public Reference {
        unsigned m_value;
        String m_protocolSNRef;

        ComParamRef(const pugi::xml_node&);
    };

    struct PhysicalDimension : public BasicInfo {
        unsigned m_lengthExp;
        unsigned m_timeExp;

        PhysicalDimension() = default;
        PhysicalDimension(const pugi::xml_node&);
    };

    struct Unit : public BasicInfo {
        String m_displayName;
        unsigned m_factorSiToUnit = 0;
        unsigned m_offsetSiToUnit = 0;
        Ref<PhysicalDimension> m_physicalDimension;

        Unit() = default;
        Unit(const pugi::xml_node&, const Map<String, Ref<PhysicalDimension>>& physDimensionMap);
    };

    enum class DCType {
        StandardLength,
        MinMaxLength,
        LeadingLengthInfo,
        ParamLengthInfo
    };


    enum class BaseTypeEncoding {
        A_UINT32,
        A_UNICODE2STRING,
        A_BYTEFIELD,
        A_INT32,
        A_ASCIISTRING,
    };

    struct DiagCodedType {
        DCType                  m_xsiType;
        BaseTypeEncoding        m_baseDataType;

        std::optional<String>   m_baseTypeEncoding;
        std::optional<bool>     m_isHighLowByteOrder;
        std::optional<String>   m_termination;

        std::optional<unsigned> m_bitLength;
        std::optional<String>   m_bitMask; // store a hex value in string
        std::optional<unsigned> m_maxLength;
        std::optional<unsigned> m_minLength;
        std::optional<String>   m_lengthKeyRef;

        DiagCodedType() = default;
        DiagCodedType(const pugi::xml_node&);
        String encode(unsigned input);
        Option<unsigned> diag_code(const String& value);
        
    };

    struct DopBase : public BasicInfo {
        bool m_isVisible;


        DopBase(const pugi::xml_node&);
        //virtual unsigned lowerLimit(const String& physicalValue) {};
        virtual Option<unsigned> encode(const String&) = 0;
        virtual String decode(const String&) = 0;
    };

    struct TextTableCompuScale {
        String   m_vt;
        String   m_ti;
        String   m_shortLabel;
        unsigned m_lowerLimit = 0;
        unsigned m_upperLimit = 0;

        TextTableCompuScale() = default;
        TextTableCompuScale(const pugi::xml_node& node);
    };

    /*
        * Possible compute method:
        * 1) Identical
        *    f(x) = x
        * 2) Linear
        *    f(x) = a * x + b
        * 3) Texttable
        *    internal    physical
        *     [0, 1)     "1 Min"
        *     [2, 5)     "2 Min"
        *     [9, ��)     "Always"
        * 4) Scale Linear
        *           �� a1 * x + b1
        *    f(x) = | a2 * x + b2
        *           �� a3 * x + b3
        * 5) Tab Interpolated
        *    internal    physical
        *     0           0
        *     2           5
        *     4           8
        *     8           10
        *     10          10
        *
        *----- The categories below are not found.
        *
        * 5) Rational Function
        *    f(x) = x^n
        * 6) Scale Rational Function
        *    For example:
        *           �� x^n, (0 <= x < 2)
        *    f(x) = |
        *           �� a * x + b, (2 < x)
        */
    struct ComputeMethod {

        //ComputeMethod(const pugi::xml_node&);
        virtual String decode(const String& value) = 0;
        virtual String decode(unsigned value) = 0;
        virtual Option<unsigned> encode(const String& value) = 0;
    };

    struct IdenticalComputeMethod : public ComputeMethod {
        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
        String decode(unsigned value);
        IdenticalComputeMethod() {};
    };

    struct LinearComputeMethod : public ComputeMethod {
        String m_description;
        double m_compuNumerators[2];
        double m_compuDenominator;

        LinearComputeMethod(const pugi::xml_node&);
        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
        String decode(unsigned value);
    };

    struct TextTableComputeMethod : public ComputeMethod {
        Map<unsigned, TextTableCompuScale> m_textTableCompuScales;

        TextTableComputeMethod(const pugi::xml_node&);
        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
        String decode(unsigned value);
    };

    struct PhysicalType {
        /*
        * A Physical Type could be one of:
        *	a) A_UINT32 { optional: DISPLAY-RADIX="HEX" }
        *	b) A_UNICODE2STRING
        *	c) A_FLOAT32 { <PRECISION>3</PRECISION> }
        *	d) A_BYTEFIELD
        *	e) A_INT32
        *	f) A_FLOAT64
        */
        String m_baseDataType;

        std::optional<String>	m_dispayRadix;
        std::optional<unsigned> m_floatPrecision;

        PhysicalType() = default;
        PhysicalType(const pugi::xml_node&);
        String encode(const String& value);
        String decode(const String& value);
    };

    struct InternalConstr {

        // lets currently leaves this out, because most of them
        // are not useful in decoding traces
        /*struct ScaleConstr {
            String m_validity;
            String m_shortLabel;
            String m_shortLabelTI;
            int m_lowerLimit = 0;
            int m_upperLimit = 0;

            ScaleConstr(const pugi::xml_node&);
        };*/

        int m_lowerLimit = 0;
        int m_upperLimit = 0;
        // currently all the types are "CLOSED", so...
        // String m_lowerLimitType;
        // String m_upperLimitType;
        //Vec<ScaleConstr> m_scaleConstrs;

        InternalConstr() = default;
        InternalConstr(const pugi::xml_node&);
    };


    struct DataObjectProp : public DopBase {
        Ref<ComputeMethod> m_compuMethod;
        DiagCodedType m_diagCodedType;
        PhysicalType m_physicalType;
        InternalConstr m_internalConstr;

        std::optional<Reference> m_unitRef;

        Ref<Unit> m_unit;

        DataObjectProp() = default;
        DataObjectProp(const pugi::xml_node& node);
        Option<unsigned> coded_value(const String& str);
        void dereference(const Map<String, Ref<Unit>>& unitMap);
        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
    };

    struct DTC : public BasicInfo {
        String m_displayTroubleCode;
        String m_text;
        int m_level;
        int m_troubleCode;

        DTC() = default;
        DTC(const pugi::xml_node&);
    };

    struct DtcDop : public DataObjectProp {
        bool m_isVisible;
        // Vec<std::variant<Ref<DTC>, Reference>> m_dtcs;
        Map<String, Ref<DTC>> m_dtcs;

        DtcDop() = delete;
        DtcDop(const pugi::xml_node&);
    };

    struct Param {
        String m_shortName;
        String m_longName;
        String m_description;

        String m_type;
        std::optional<String> m_semantic;
        std::optional<unsigned> m_bytePosition;
        unsigned m_bitPosition = 0;

        Param(const pugi::xml_node&);
        virtual String decode(const String& text) = 0;
        virtual String encode(const String& text) = 0;
    };

    struct DiagLayerContainer;
    struct ParamWithDop : public Param {
        std::optional<Reference> m_dopRef = std::nullopt;
        std::optional<DopSNRef> m_dopSNRef = std::nullopt;
        Ref<DopBase> m_dop = nullptr;

        ParamWithDop(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
    private:
        void dereference(DiagLayerContainer* parentDlc);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct CodedConstParam : public Param {
        DiagCodedType m_diagCodedType;
        unsigned m_codedValue;

        CodedConstParam(const pugi::xml_node&);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct ReservedParam : public Param {
        unsigned m_bitLength;
        DiagCodedType m_diagCodedType;

        ReservedParam(const pugi::xml_node&);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct MatchingRequestParam : public Param {
        unsigned m_requestBytePos;
        unsigned m_byteLength;

        MatchingRequestParam(const pugi::xml_node&);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct ValueParam : public ParamWithDop {
        String m_physicalDefaultValue;

        ValueParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct PhysConstParam : public ParamWithDop {
        String m_physConstantValue;

        PhysConstParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
        String decode(const String& text) override;
        String encode(const String& text) override;

    private:
        unsigned m_constLowerLimit = 0;
    };

    struct LengthKeyParam : public ParamWithDop {
        String m_id;

        LengthKeyParam(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct StructBase : public DopBase {

        Vec<Ref<Param>> m_params;

        StructBase() = default;
        StructBase(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
        String decode(const String& text) override;
        Option<unsigned> encode(const String& text) override;
    };

    struct Structure : public StructBase {
        unsigned m_byteSize;

        Structure() = default;
        Structure(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
    };

    struct Table : public BasicInfo {
        struct TableRow : public BasicInfo {
            String m_key;
            Reference m_structureRef;

            Ref<Structure> m_structure;  // after dereference

            TableRow() = default;
            TableRow(const pugi::xml_node& node, const Map<String, Ref<Structure>>& structMap);
        };

        String m_semantic;
        Ref<DataObjectProp> m_keyDop;
        // table rows must be constructed in order
        Vec<std::variant<Ref<TableRow>, Reference>> m_tableRows;

        Table() = default;
        Table(const pugi::xml_node& node,
            const Map<String, Ref<DataObjectProp>>& dopMap,
            const Map<String, Ref<Structure>>& structMap);
    };

    struct TableKeyParam : public Param {
        String m_id;
        Reference m_tableRef;
        Ref<Table> m_table;

        TableKeyParam(const pugi::xml_node& node);
        void dereference(const Map<String, Ref<Table>>& tableMap);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct TableStructParam : public Param {
        Reference m_tableKeyRef;
        Ref<TableKeyParam> m_tableKey;

        // TableStructParam seems always reference to  TableKeyParam in the same structure
        TableStructParam(const pugi::xml_node& node);
        void dereference(Ref<StructBase> parentStuct);
        String decode(const String& text) override;
        String encode(const String& text) override;
    };

    struct Request : public StructBase {
        Request(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
    };

    struct Response : public StructBase {

        Response(const pugi::xml_node& node, DiagLayerContainer* parentDlc);
    };

    struct StaticField : public BasicInfo {
        Ref<Structure> m_basicStructure;
        unsigned m_fixedNumberOfItems;
        unsigned m_itemByteSize;

        StaticField(const pugi::xml_node& node, const Map<String, Ref<Structure>>& structMap);
    };

    struct DynamicLengthField : public DopBase {
        struct DeterminNumberOfItems {
            unsigned m_bytePosition = 0;
            unsigned m_bitPosition = 0;
            Reference m_dataObjectPropRef;

            Ref<DataObjectProp> m_dataObjectProp;

            DeterminNumberOfItems() = default;
            DeterminNumberOfItems(const pugi::xml_node&, const Map<String, Ref<DataObjectProp>>&);
        };


        bool m_isVisible;
        Reference m_basicStructureRef;
        unsigned m_offset = 0;
        DeterminNumberOfItems m_determinNumberOfItems;

        Ref<Structure> m_basicStructure;

        DynamicLengthField() = default;
        DynamicLengthField(const pugi::xml_node&, const Map<String, Ref<Structure>>&, const Map<String, Ref<DataObjectProp>>&);
        void dereference(const Map<String, Ref<Structure>>&, const Map<String, Ref<DataObjectProp>>&);
        String decode(const String& text) override;
        Option<unsigned> encode(const String& text) override;
    };


    struct EndOfPduField : public DopBase {
        Ref<Structure> m_basicStruct;
        unsigned m_maxItems;
        unsigned m_minItems;
        bool m_isVisible = true;
    public:
        EndOfPduField() = default;
        EndOfPduField(const pugi::xml_node&, const Map<String, Ref<Structure>>&);
        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
    };

    struct Mux : public DopBase {
        struct SwitchKey {
            unsigned m_bytePosition = 0;
            unsigned m_bitPosition = 0;
            Reference m_dopRef;

            SwitchKey() = default;
            SwitchKey(const pugi::xml_node&);
        };

        struct MuxCase {
            String m_shortName;
            String m_longName;
            Reference m_strutureRef;
            unsigned m_lowerLimit = 0;
            unsigned m_upperLimit = 0;

            MuxCase() {};
            MuxCase(const pugi::xml_node&);
        };

        bool m_isVisible;
        unsigned m_bytePosition;
        SwitchKey m_switchKey;
        MuxCase m_defaultCase;
        Vec<MuxCase> m_cases;

        Mux() = default;
        Mux(const pugi::xml_node&);

        Option<unsigned> encode(const String& value) override;
        String decode(const String& value) override;
    };



    struct FunctClass : public BasicInfo {
        FunctClass(const pugi::xml_node&);
    };

    struct Audience {
        bool isSupplier;
        bool isDevelopment;
        bool isManufacturing;
        bool isAfterSales;
    };


    struct DiagService : public BasicInfo {
        String m_addressing;
        String m_semantic;
        // Vec<Reference> m_functClassRefs;
        Reference m_requestRef;
        Reference m_posResponseRef;
        Reference m_negResponseRef;
        // Audience m_audience;
        Ref<Request> m_request = nullptr;
        Ref<Response> m_posResponse = nullptr;
        Ref<Response> m_negResponse = nullptr;

    public:
        bool decode(Trace& trace);
        int serviceId();
        DiagService(const pugi::xml_node&);
        void dereference(DiagLayerContainer* dlc);

    private:
        bool m_derefed{ false };
        int m_serviceId{ -1 };
    };

    struct SingleEcuJob : public BasicInfo {
        struct ProgCode {
            String m_codeFile;
            String m_syntax;
            String m_revision;
            String m_encryption;
        };

        struct InputParam {
            String m_shorName;
            String m_longName;
            String m_physicalDefaultValue;
            Reference m_dopBaseRef;
        };

        // Vec<Reference> m_functClassRefs;
        // Audience m_audience;
        Vec<ProgCode> m_progCodes;
        Vec<InputParam> m_inputParams;

        SingleEcuJob() = default;
        SingleEcuJob(const pugi::xml_node&);
    };

    struct EcuVariantPattern {
        struct MatchingParameter {
            String m_expectedValue;
            String m_diagCommSnref;
            String m_outParamIfSnref;
        };

        Vec<MatchingParameter> m_matchingParameters;

        EcuVariantPattern() = default;
        EcuVariantPattern(const pugi::xml_node&);
    };


    // using DiagComm = std::variant<Ref<DiagService>, Ref<SingleEcuJob>, void*>;

    struct DiagComm {
        String shortName;
        DiagCommType type;
        Ref<DiagService> diagService;
        Ref<SingleEcuJob> singleEcuJob;
    };

    // ---------------------------
    struct ParentRef : public Reference {
        ParentRefType m_type;
        Set<String> notInheritedDiagComms;
        //Ref<DiagLayerContainer> parentDlc;

        ParentRef() = default;
        //ParentRef(const ParentRef& rhs);
        ParentRef(const pugi::xml_node& node);

        bool operator<(const ParentRef& other) const {
            return m_type < other.m_type;
        }

        bool operator==(const ParentRef& other) const {
            return idRef == other.idRef;
        }
    };
    // ---------------------------

    struct DiagLayerContainer : public BasicInfo {



        String m_containerType;

        // Diag Data Dictionary Specifications
        Map<String, Ref<Unit>>					m_units;
        Map<String, Ref<PhysicalDimension>>		m_physicalDimensions;
        Map<String, Ref<DtcDop>>				m_dtcDops;
        Map<String, Ref<DataObjectProp>>		m_dataObjectProps;
        Map<String, Ref<Structure>>				m_structures;
        Map<String, Ref<StaticField>>			m_staticFields;
        Map<String, Ref<DynamicLengthField>>	m_dynamicLengthFields;
        Map<String, Ref<EndOfPduField>>			m_endOfPduFields;
        Map<String, Ref<Mux>>					m_muxes;
        Map<String, Ref<Table>>					m_tables;

        // Map<String, Ref<FunctClass>>			m_funcClasses;
        // A DiagComm could be:
        Map<String, DiagComm>					m_diagComms;
        Map<String, Ref<DiagService>>			m_diagServices;
        Map<String, Ref<SingleEcuJob>>			m_singleEcuJobs;
        Map<String, Ref<Request>>				m_requests;
        Map<String, Ref<Response>>				m_posResponses;
        Map<String, Ref<Response>>				m_negResponses;
        Map<String, Ref<Response>>				m_globalNegResponses;

        //Vec<ComParamRef>						m_comparamRefs;
        //Vec<Reference>							m_importRefs;
        Vec<ParentRef>					        m_parentRefs;
        //Reference								m_comParamSpecRef;

        // Vec<EcuVariantPattern>					m_ecuVariantPatterns;

        bool m_parentLoaded = false;
        void inherit();
        void dereference();

    private:
        //Map<String, Ref<DiagLayerContainer>>	m_referencedDlcs;
        //Vec<String>								m_subEcuVariants;


    public:
        DiagLayerContainer(const String& dlcName);

        void decode(Trace& trace);
        void decodeEcuTrace(EcuTrace& ecuTrace);
        void inheritDiagServices();

        Ref<FunctClass>			getFunctClassById(const String&) const;
        Ref<Request>			getRequestById(const String&) const;
        Ref<Response>			getResponseById(const String&) const;
        Ref<DiagService>		getDiagServiceById(const String& id) const;
        Ref<DataObjectProp>		getDataObjectPropById(const String& id) const;
        Ref<DtcDop>				getDtcDopById(const String& id) const;
        Ref<DopBase>			getDopById(const String& id) const;
        Ref<Structure>			getStructureById(const String& id) const;
        Ref<EndOfPduField>		getEndOfPduFieldById(const String& id) const;
        Ref<Mux>			    getMuxById(const String& id) const;
        Ref<DynamicLengthField>	getDynamicLengthFieldById(const String& id) const;
        Ref<Table>				getTableById(const String& id) const;
        Ref<Unit>				getUnitById(const String& id) const;
        Ref<DTC>				getDtcById(const String& id) const;
        Ref<PhysicalDimension>	getPhysicalDimensionById(const String& id) const;
        Ref<DataObjectProp>		getDopByShortName(const String& shortName) const;
        DiagComm				getDiagCommById(const String& id) const;
        Vec<ParentRef>          getAllParentRefs() const;

        const Map<String, DiagComm>& getAllDiagComms() const;
        Vec<Ref<DiagService>> getAllDiagServices();
        //const Vec<String>& getSubEvShortNames(const String&) const;
        Set<Ref<DiagService>> getDiagServicesByValue(unsigned value) const;

        const String& shortName() const {
            return m_shortName;
        }
        const String& id() const {
            return m_id;
        }
    };

    struct BasicVariant : BasicInfo {
    };

    struct LogicalLink : BasicInfo {
        String m_physicalVehicleLink;
        Reference m_protocolRef;
        Reference m_functionalGroupRef;
        std::optional<Reference> m_baseVariantRef;

        Vec<Reference> m_linkComParamRefs;

        LogicalLink() = default;
        LogicalLink(const pugi::xml_node&);
        String shortName() const;
        std::optional<String> getBaseVariant() const {
            if (m_baseVariantRef.has_value()) {
                return m_baseVariantRef->idRef;
            }
            return std::nullopt;
        }
    };

    struct VehicleInformation : public BasicInfo {
        Vec<LogicalLink> m_logicalLinks;

        VehicleInformation() = default;
        VehicleInformation(const pugi::xml_node&);
        std::string shortName() const;
        Vec<String> getLogicalLinkShortNames();
        Vec<String> getBvShortNames();
        const Vec<LogicalLink>& getLogicalLinks() {
            return m_logicalLinks;
        }
    };

    struct UnitGroup : public BasicInfo {
        String m_category;
        Vec<Reference> m_unitRefs;

        UnitGroup() = default;
        UnitGroup(const pugi::xml_node&);
    };

    struct ComParamSpec;

    struct ComParam : public BasicInfo {

        unsigned m_displayLevel;
        String m_cpType;
        String m_paramClass;
        String m_physicalDefaultValue;
        Ref<DataObjectProp> m_dop;

        ComParam(const pugi::xml_node&, const ComParamSpec*);
    };


    struct ComParamSpec : public BasicInfo {

        Map<String, Ref<PhysicalDimension>> m_physicalDimensions;
        Map<String, Ref<Unit>> m_units;
        Map<String, Ref<ComParam>> m_comparams;
        Map<String, Ref<DataObjectProp>> m_dataObjProps;
    public:
        ComParamSpec() = default;
        ComParamSpec(const std::string&);
    };

    struct UnitSpec {
        Map<String, Ref<UnitGroup>>				m_unitGroups;
        Map<String, Ref<Unit>>					m_units;
        Map<String, Ref<PhysicalDimension>>		m_physicalDimensions;

        UnitSpec(const pugi::xml_node&);
    };

    class PDX
    {
    public:
        PDX(const PDX&) = delete;

        void init(std::filesystem::path path);

        static PDX& get() {
            return s_instance;
        }

        static bool isLoaded() {
            return m_loaded;
        }

        void decodeEcuTrace(EcuTrace& ecuTrace);
        Vec<String> getEvShortNamesByBvId(const String& id);
        Vec<String> getBvShortNamesByVehicleInfoId(const String& id);
        Vec<String> getLogicalLinksByVehicleInfoId(const String& id);
        Vec<String> getVehicleInfoShortNames() const;
        Ref<VehicleInformation> getVehicleInfoById(const String& id);
        String getDlcFileNameById(const String&);
        Ref<DiagLayerContainer> getEvByShortName(const String&);
        Ref<DiagLayerContainer> getDlcById(const String& id);
        Ref<ComParamSpec> getComParamSpecById(const String& id);
        Ref<DiagLayerContainer> addDlcById(const String& id);
        Ref<DopBase> getDopByDocAndId(const String& doc, const String& id);
        Ref<DataObjectProp> getDataObjectPropByDocAndId(const String& doc, const String& id);
        Ref<Structure> getStructureByDocAndId(const String& doc, const String& id);
        Ref<Unit> getUnitByDocAndId(const String& doc, const String& id);
        Ref<DTC> getDtcByDocAndId(const String& doc, const String& id);
        Ref<Table> getTableByDocAndId(const String& doc, const String& id);
        Ref<PhysicalDimension> getPhysicalDimensionByDocAndId(const String& doc, const String& id);
        DiagComm getDiagCommByDocAndId(const String& doc, const String& id);
        Ref<DiagService> getDiagServiceByDocAndId(const String& doc, const String& id);

    private:
        PDX() {}
        void initDLCByShortName(const String&);

        String m_modelYear;
        String m_oem;
        String m_vehicleModel;
        String m_visFile;

        Vec<String> cps;
        Vec<String> m_bvs;
        Map<int, String> m_canIdMapBv;
        Map<String, Vec<String>> m_bvMapSubEvs;
        Map<String, Ref<DiagLayerContainer>> m_dlcMap;
        Map<String, Ref<ComParamSpec>> m_comparamSpecs;
        Map<String, Ref<VehicleInformation>> m_vehicleInfoMap;
        Map<String, String> m_dlcFiles;
        static PDX s_instance;

        inline static bool m_loaded;
    };

    String hexToString(const String& hex);
}