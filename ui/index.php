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

function getSortedSizeData(array $sizeData)
{
    uasort($sizeData, function($itemA, $itemB) {
        $fullSizeA = $itemA['full_size'];
        $fullSizeB = $itemB['full_size'];

        if ($fullSizeA === $fullSizeB) {
            return 0;
        }

        return ($fullSizeA < $fullSizeB) ? 1 : -1;
    });

    return $sizeData;
}

function getTopConsumers(array $sizeData, $limit)
{
    $sortedSizeData = getSortedSizeData($sizeData);

    return array_slice($sortedSizeData, 0, $limit);
}

function showTopConsumers(array $sizeData, $meminfoFile)
{
    $topConsumers = getTopConsumers($sizeData, 100);
    echo "<h2>Top Consumers</h2>\n";
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
        printf ("<td>%s</td>\n", "TODO");
        echo "</tr>\n";
    }
    echo "</table>\n";
}

function showItemId(array $sizeData, $meminfoFile, $itemId)
{
    $itemData = $sizeData[$itemId];

    printf ("<h2>Item %s</h2>", $itemId);

    echo "<h3>Information</h3>";

    echo "TODO";

    echo "<h3>Parents</h3>";

    echo "TODO";

    echo "<h3>Children</h3>";

    echo "<table>\n";
    echo "<tr>\n";
    echo "<th>Array Key/Object Attribute</th><th>Item id</th><th>Item type</th><th>Object class</th><th>Total size</th><th>Self size</th><th>Children count</th><th>Child of</th>\n";
    echo "</tr>\n";

    foreach ($itemData['children'] as $childKey => $childId) {
        $childData = $sizeData[$childId];

        echo "<tr>\n";
        printf ("<td>%s</td>", $childKey);
        printf (
            "<td><a href='?file=%s&item_id=%s'>%s</a></td>\n",
            $meminfoFile,
            $childId,
            $childId
        );
        printf ("<td>%s</td>\n", $childData['type']);
        $class = isset($childData['class'])?$childData['class']:"";
        printf ("<td>%s</td>\n", $class);
        printf ("<td>%s</td>\n", $childData['full_size']);
        printf ("<td>%s</td>\n", $childData['size']);
        printf ("<td>%s</td>\n", count($childData['children']));
        printf ("<td>%s</td>\n", "TODO");
        echo "</tr>\n";
    }
    echo "</table>\n";
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

if (isset($_GET['item_id'])) {
    $itemId = $_GET['item_id'];
    showItemId($sizeData, $meminfoFile, $itemId);
} else {
    showTopConsumers($sizeData, $meminfoFile);
}


