<?php
function computeFullSize($itemId, array $itemData, array &$sizeData, array &$visitedItems)
{
    $size = $itemData['size'];

    if (isset($itemData['children'])) {
        foreach($itemData['children'] as $key => $childId) {
            if (!isset($visitedItems[$childId])) {
                $visitedItems[$childId] = true;

                if (!isset($sizeData[$childId])) {
                    //echo "Unable to find $childId. Should be there, problem at memory dumping\n";
                } else {
                    $size += computeFullSize($childId, $sizeData[$childId], $sizeData, $visitedItems);
                    $sizeData[$childId]['accounted_in'] = $itemId;
                }
            }
        }
    }

    $sizeData[$itemId]['full_size'] = $size;

    return $size;
}

function appendFullSize(array &$sizeData)
{
    $visitedItems = array();

    foreach ($sizeData as $itemId => $itemData) {
        if (!isset($sizeData[$itemId]['full_size'])) {
            $sizeData[$itemId]['full_size'] = computeFullSize($itemId, $itemData, $sizeData, $visitedItems);
        }
    }
}

function appendParents(array &$sizeData)
{
    foreach ($sizeData as $itemId => $itemData) {
        if (isset($sizeData[$itemId]['children'])) {
            foreach ($sizeData[$itemId]['children'] as $index => $childId) {
                $sizeData[$childId]['parents'][$itemId."::".$index] = $itemId;
            }
        }
    }
}

function compareFullSize(array $itemA, array $itemB)
{
    $fullSizeA = $itemA['full_size'];
    $fullSizeB = $itemB['full_size'];

    if ($fullSizeA === $fullSizeB) {
        return 0;
    }

    return ($fullSizeA < $fullSizeB) ? 1 : -1;
}

function getTopConsumers(array $sizeData, $limit)
{
    uasort($sizeData, "compareFullSize");

    return array_slice($sizeData, 0, $limit);
}

function showTopConsumers(array $sizeData, $meminfoFile, $maxTopConsumers = 100)
{
    $topConsumers = getTopConsumers($sizeData, $maxTopConsumers);
    echo "<html>\n";
    echo "<head>\n";
    printf("<title>Summary for %s</title>\n", $meminfoFile);
    echo "</head>\n";

    echo "<body>\n";
    printf("<h1>Summary for memory info %s</h1>\n", $meminfoFile);

    echo "<h2>Metadata</h2>\n";

    echo "<h2>Classes info</h2>\n";

    printf("<h2>Top %s Consumers</h2>\n", $maxTopConsumers);
    echo "<table>\n";
    echo "<tr>\n";
    echo "<th>Item id</th><th>Item type</th><th>Object class</th><th>Total size</th><th>Self size</th><th>Children count</th><th>Child of</th>\n";
    echo "</tr>\n";

    foreach ($topConsumers as $itemId => $itemData) {
        echo "<tr>\n";
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $itemId,
            $itemId
        );
        printf ("<td>%s</td>\n", $itemData['type']);
        $class = isset($itemData['class'])?$itemData['class']:"";
        printf ("<td>%s</td>\n", $class);
        printf ("<td>%s</td>\n", $itemData['full_size']);
        printf ("<td>%s</td>\n", $itemData['size']);
        printf ("<td>%s</td>\n", count($itemData['children']));
        printf ("<td>%s</td>\n", getParentsList($itemData));
        echo "</tr>\n";
    }
    echo "</table>\n";
    echo "</body>\n";
    echo "</html>\n";
}

function getParentsList(array $itemData, $maxParents = 5)
{
    $parentsList = implode(
        ',',
        array_slice(
            array_keys(
                $itemData['parents']
            ),
            0,
            $maxParents
        )
    );

    if (count($itemData['parents']) > $maxParents) {
        $parentsList .= ', ...';
    }

    return $parentsList;
}

function showItemId(array $sizeData, $meminfoFile, $itemId)
{
    $itemData = $sizeData[$itemId];

    echo "<html>\n";
    echo "<head>\n";
    printf ("<title>%s %s</title>", $itemData['type'], $itemId);
    echo "</head>\n";

    echo "<body>\n";

    printf ('<p><a href="?file=%s">Go back to summary</a></p>', $meminfoFile);

    printf ("<h1>%s %s</h1>", $itemData['type'], $itemId);

    echo "<h2>Information</h2>";
    echo "<ul>";
    if (isset($itemData['class'])) {
        printf ('<li>Class: %s</li>', $itemData['class']);
    }
    printf ('<li>Self Size: %s</li>', $itemData['size']);
    printf ('<li>Accounted In: %s</li>', $itemData['accounted_in']);
    printf ('<li>Full Size: %s</li>', $itemData['full_size']);

    echo "</ul>";

    echo "<h2>Parents</h2>";

    echo "<table>\n";
    echo "<tr>\n";
    echo "<th>Key</th><th>Item id</th><th>Item type</th><th>Object class</th><th>Total size</th><th>Self size</th><th>Children count</th><th>Child of</th>\n";
    echo "</tr>\n";

    foreach ($itemData['parents'] as $parentKey => $parentId) {
        $parentData = $sizeData[$parentId];

        echo "<tr>\n";
        printf ("<td>%s</td>", $parentKey);
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $parentId,
            $parentId
        );
        printf ("<td>%s</td>\n", $parentData['type']);
        $class = isset($parentData['class'])?$parentData['class']:"";
        printf ("<td>%s</td>\n", $class);
        printf ("<td>%s</td>\n", $parentData['full_size']);
        printf ("<td>%s</td>\n", $parentData['size']);
        printf ("<td>%s</td>\n", count($parentData['children']));
        printf ("<td>%s</td>\n", getParentsList($parentData));
        echo "</tr>\n";
    }
    echo "</table>\n";

    $accountedChildren = [];
    $nonAccountedChildren = [];

    foreach ($itemData['children'] as $childKey => $childId) {
        $childData = $sizeData[$childId];
        $childData['item_id'] = $childId;

        if ($childData['accounted_in'] == $itemId) {
            $accountedChildren[$childKey] = $childData;
        } else {
            $nonAccountedChildren[$childKey] = $childData;
        }
    }

    uasort($accountedChildren, "compareFullSize");
    uasort($nonAccountedChildren, "compareFullSize");

    echo "<h2>Accounted Children</h2>";

    echo "<table>\n";
    echo "<tr>\n";
    echo "<th>Key</th><th>Item id</th><th>Item type</th><th>Object class</th><th>Total size</th><th>Self size</th><th>Children count</th><th>Child of</th>\n";
    echo "</tr>\n";

    foreach ($accountedChildren as $childKey => $childData) {
        echo "<tr>\n";
        printf ("<td>%s</td>", $childKey);
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $childData['item_id'],
            $childData['item_id']
        );
        printf ("<td>%s</td>\n", $childData['type']);
        $class = isset($childData['class'])?$childData['class']:"";
        printf ("<td>%s</td>\n", $class);
        printf ("<td>%s</td>\n", $childData['full_size']);
        printf ("<td>%s</td>\n", $childData['size']);
        printf ("<td>%s</td>\n", count($childData['children']));
        printf ("<td>%s</td>\n", getParentsList($childData));
        echo "</tr>\n";
    }
    echo "</table>\n";

    echo "<h2>Non Accounted Children</h2>";

    echo "<table>\n";
    echo "<tr>\n";
    echo "<th>Key</th><th>Item id</th><th>Item type</th><th>Object class</th><th>Total size</th><th>Accounted in</th><th>Self size</th><th>Children count</th><th>Child of</th>\n";
    echo "</tr>\n";

    foreach ($nonAccountedChildren as $childKey => $childData) {
        echo "<tr>\n";
        printf ("<td>%s</td>", $childKey);
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $childData['item_id'],
            $childData['item_id']
        );
        printf ("<td>%s</td>\n", $childData['type']);
        $class = isset($childData['class'])?$childData['class']:"";
        printf ("<td>%s</td>\n", $class);
        printf ("<td>%s</td>\n", $childData['full_size']);
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $childData['accounted_in'],
            $childData['accounted_in']
        );
        printf ("<td>%s</td>\n", $childData['size']);
        printf ("<td>%s</td>\n", count($childData['children']));
        printf ("<td>%s</td>\n", getParentsList($childData));
        echo "</tr>\n";
    }
    echo "</table>\n";

    echo "</body>\n";
    echo "</html>\n";
}

if (!isset($_GET['file'])) {
    die('Please specifiy the file to analyze with the "file" parameter');
}

$meminfoFile = $_GET['file'];

if (!file_exists($meminfoFile)) {
    die("Unable to find $meminfoFile");
}

$sizeData = json_decode(file_get_contents($meminfoFile), true);
appendFullSize($sizeData);
appendParents($sizeData);

if (isset($_GET['item_id'])) {
    $itemId = $_GET['item_id'];
    showItemId($sizeData, $meminfoFile, $itemId);
} else {
    showTopConsumers($sizeData, $meminfoFile);
}


