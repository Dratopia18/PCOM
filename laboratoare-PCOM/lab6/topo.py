#!/usr/bin/env python

"""
The example topology creates a router and three IP subnets:

    - 192.168.1.0/24 (r0-eth1, IP: 192.168.1.1)
    - 172.16.0.0/12 (r0-eth2, IP: 172.16.0.1)
    - 10.0.0.0/8 (r0-eth3, IP: 10.0.0.1)

Each subnet consists of a single host connected to
a single switch:

    r0-eth1 - s1-eth1 - h1-eth0 (IP: 192.168.1.100)
    r0-eth2 - s2-eth1 - h2-eth0 (IP: 172.16.0.100)
"""


from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.link import TCLink
import sys

class LinuxRouter( Node ):
    "A Node with IP forwarding enabled."

    # pylint: disable=arguments-differ
    def config( self, **params ):
        super( LinuxRouter, self).config( **params )
        # Enable forwarding on the router
        self.cmd( 'sysctl net.ipv4.ip_forward=1' )

    def terminate( self ):
        self.cmd( 'sysctl net.ipv4.ip_forward=0' )
        super( LinuxRouter, self ).terminate()


class NetworkTopo( Topo ):
    "A LinuxRouter connecting three IP subnets"


    def build( self, **_opts ):

        defaultIP = '192.168.1.1/24'  # IP address for r0-eth1
        router = self.addNode( 'r0', cls=LinuxRouter, ip=defaultIP) 


        h1 = self.addHost( 'h1', ip='192.168.1.100/24',
                           defaultRoute='via 192.168.1.1' )

        h2 = self.addHost( 'h2', ip='172.16.0.100/12',
                           defaultRoute='via 172.16.0.1' )

        # 10 Mbps, 10ms delay, 10% packet loss
        self.addLink( h1, router, intfName1='r0-eth1', bw=10, delay='10ms', loss=10,
                     params1={ 'ip' : '192.168.1.1/24' })  # for clarity

        # 10 Mbps, 10ms delay, 10% packet loss
        self.addLink( h2, router, intfName2='r0-eth2', bw=10, delay='10ms', loss=10,
                      params2={ 'ip' : '172.16.0.1/12' } )

class NetworkManager(object):
    def __init__(self, net):
        self.h1 = net.get("h1")
        self.h2 = net.get("h2")


def run():
    "Test linux router"
    topo = NetworkTopo()
    net = Mininet( topo=topo, link=TCLink,
                   waitConnected=True, controller=None)  # controller is used by s1-s3
    net.start()
    #print( '*** Routing Table on Router:\n' )
    #print(net[ 'r0' ].cmd( 'route' ))

    net.startTerms()
    CLI( net )
    net.stop()


if __name__ == '__main__':
    setLogLevel('critical')
    run()
