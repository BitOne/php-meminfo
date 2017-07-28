<?php

class MyObject
{
    public $myPublicMember = "Value of myPublicMember";
    protected $myProtectedMember = "Value of myProtectedMember";
    private $myPrivateMember = "Value of myPrivateMember";

}

$myObject = new MyObject();

meminfo_info_dump(fopen('php://stdout', 'w'));

print_r($GLOBALS);
