<?php

$books = new StdClass();

$books->title =  "My lists";
$books->chapter = [
    "title" => "My books",
    "para" => [
        "informaltable" => [
            "tgroup" => [
                "thead" => [
                    "Title",
                    "Author",
                    "Language",
                    "ISBN"
                ],
                "tbody" => [
                    [
                        "The Grapes of Wrath",
                        "John Steinbeck",
                        "en",
                        "0140186409"
                    ],
                    [
                        "The Pearl",
                        "John Steinbeck",
                        "en",
                        "014017737X"
                    ],
                    [
                        "Samarcande",
                        "Amine Maalouf",
                        "fr",
                        "2253051209"
                    ]
                ]
            ]
        ]
    ]
];
