#!/bin/bash

cd "$(dirname "$0")"

CLASS_NAME=$1
DIRECTORY="src"
INC_DIR="src"
if [ -z "${CLASS_NAME}" ] || [ -z "${DIRECTORY}" ]
then
	echo "Error. Please:"
	echo " - 1st parameter: Class name."
        exit 1
fi

CLASS_HEADER="${INC_DIR}/${CLASS_NAME}.h"
CLASS_SOURCE="${DIRECTORY}/${CLASS_NAME}.cpp"
if [ -f "${CLASS_HEADER}" ]
then
	echo "Error. File '${CLASS_HEADER}' exists in directory '${INC_DIR}'"
	exit 3
fi

if [ -f "${CLASS_SOURCE}" ]
then
	"Error. File '${CLASS_SOURCE}' exists in directory '${DIRECTORY}'"
	exit 4
fi

CLASS_INC="$(echo "${CLASS_NAME}" | tr /a-z/ /A-Z/)_H"

echo "\
#ifndef ${CLASS_INC}
#define ${CLASS_INC}

#include \"Bang/Bang.h\"
#include \"BangEditor/BangEditor.h\"

using namespace Bang;

class ${CLASS_NAME}
{
public:
    ${CLASS_NAME}();
    virtual ~${CLASS_NAME}();
};

#endif // ${CLASS_INC}
" > "./${CLASS_HEADER}"

echo "\
#include \"${CLASS_NAME}.h\"

using namespace Bang;

${CLASS_NAME}::${CLASS_NAME}()
{
}

${CLASS_NAME}::~${CLASS_NAME}()
{
}
" > "./${CLASS_SOURCE}"

echo "Class header has been created in '${CLASS_HEADER}'"
echo "Class source has been created in '${CLASS_SOURCE}'"    
