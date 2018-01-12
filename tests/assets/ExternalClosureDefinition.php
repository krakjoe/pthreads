<?php

    class ExternalClosureDefinition extends \Threaded {
        public function load() {
            $sync = function () {
                var_dump('Hello World');
            };
            $sync();
        }
    }