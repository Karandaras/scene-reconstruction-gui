#pragma once
#include <fstream>

namespace SceneReconstruction {
/** @class KIDGraph "kidgraph.h"
 *  Simple structure to contruct and store the KID Graph that
 *  allows marking and unmarking of nodes.
 *  @author Bastian Klingen
 */
  class KIDGraph {
    public:
      /** simple structure for the edges of the KID Graph
       */
      struct KIDEdge {
        /** node from which the edge comes */
        std::string from;
        /** node to which the edge points */
        std::string to;
        /** label of the edge */
        std::string label;
        /** equality operator
         *  @param rhs an edge to compare with
         *  @return true if edges are equal
         */
        bool operator==(const KIDEdge& rhs) const
        {
            if(from != rhs.from)
              return false;
            else if(to != rhs.to)
              return false;
            else if(label != rhs.label)
              return false;
            else
              return true;
        }
        /** equality operator
         *  @param rhs string representation of an edge to compare with
         *  @return true if edges are equal
         */
        bool operator==(const std::string& rhs) const
        {
            if(toString() != rhs)
              return false;
            else
              return true;
        }
        /** get the string representation of the edge
         *  @return string representation of this edge
         */
        std::string toString() const
        {
          return "\""+from+"\" --"+(label!=""?"\""+label+"\"":"")+"--> \""+to+"\"";
        }

        /** get the dot representation of the edge's style
         *  @param marked true to mark the edge (bold, red)
         *  @return string representation of this edge
         */
        std::string dot_edge(bool marked) {
          std::string style = "";
          if(!label.empty() || marked) {
            style += " [dir=back";
              if(!label.empty())
                style += ",label=\""+label+"\"";
              if(marked)
                style += ",style=bold,color=red2";
            style += "]";
          }

          return style;
        }

      };

      /** simple structure for the nodes of the KID Graph
       */
      struct KIDNode {
        /** name of the node */
        std::string node;
        /** list of pointers to parent nodes */
        std::list<KIDNode*> parents;
        /** list of pointers to child nodes */
        std::list<KIDNode*> children;

        /** equality operator
         *  only checks for names since the graph does not allow two
         *  nodes with equal names
         *  @param rhs node name
         *  @return true if names are equal
         */
        bool operator==(const std::string& rhs) const
        {
            if(node != rhs)
              return false;
            else
              return true;
        }

        /** get the name of the node
         *  @return name of the node
         */
        std::string toString() const
        {
          return node;
        }
      };

    private:
      /** marks the parents of an node
       *  @param node pointer to a node
       */
      void mark_parents(KIDGraph::KIDNode* node) {
        std::list<KIDGraph::KIDNode*>::iterator iter;
        for(iter = node->parents.begin(); iter != node->parents.end(); iter++) {
          if(!is_marked((*iter)->node)) {
            marked_nodes.push_back((*iter)->node);
            mark_parents(*iter);
          }
        }
      }

      /** marks the children of an node
       *  @param node pointer to a node
       */
      void mark_children(KIDNode* node) {
        std::list<KIDNode*>::iterator iter;
        for(iter = node->children.begin(); iter != node->children.end(); iter++) {
          if(!is_marked((*iter)->node)) {
            marked_nodes.push_back((*iter)->node);
            mark_children(*iter);
          }
        }
      }

      /** unmarks the children of an node (if not otherwise marked)
       *  @param node pointer to a node
       */
      void unmark_children(KIDNode* node) {
        std::list<KIDNode*>::iterator iter;
        for(iter = node->children.begin(); iter != node->children.end(); iter++) {
          if(is_marked((*iter)->node)) {
            bool unmark = true;
            for(std::list<KIDNode*>::iterator parent = (*iter)->parents.begin(); parent != (*iter)->parents.end(); parent++) {
              if(is_marked((*parent)->node)) {
                unmark = false;
                break;
              }
            }
            if(unmark) {
              marked_nodes.remove((*iter)->node);
              unmark_children(*iter);
            }
          }
        }
      }

      /** unmarks the parents of an node (if not otherwise marked)
       *  @param node pointer to a node
       */
      void unmark_parents(KIDGraph::KIDNode* node) {
        std::list<KIDGraph::KIDNode*>::iterator iter;
        for(iter = node->parents.begin(); iter != node->parents.end(); iter++) {
          if(is_marked((*iter)->node)) {
            bool unmark = true;
            for(std::list<KIDNode*>::iterator child = (*iter)->children.begin(); child != (*iter)->children.end(); child++) {
              if(is_marked((*child)->node)) {
                unmark = false;
                break;
              }
            }

            if(unmark) {
              marked_nodes.remove((*iter)->node);
              unmark_parents(*iter);
            }
          }
        }
      }

      
    public:
      /** save the graph to a dgf file 
       *  @param filename name of the file to save to
       */
      void save(std::string filename) {
        std::ofstream out;
        out.open(filename.c_str(), std::ios::out | std::ios::trunc);
        out << name << "\n";
        std::list<KIDNode>::iterator iter;
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";

        std::list<KIDEdge>::iterator eiter;
        for(eiter = edges.begin(); eiter != edges.end(); eiter++) {
          out << eiter->from << ";" << eiter->to << ";" << eiter->label << ";\n";
        }

        out.close();
      }
        
      /** save the graph to string 
       */
      std::string save_to_string() {
        std::stringstream out;
        std::list<KIDNode>::iterator iter;
        std::list<std::string>::iterator miter;
        out << name << "\n";
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++)
          out << iter->node << ";";
        out << "\n";

        std::list<KIDEdge>::iterator eiter;
        for(eiter = edges.begin(); eiter != edges.end(); eiter++) {
          out << eiter->from << ";" << eiter->to << ";" << eiter->label << ";\n";
        }

        if(marked_nodes.size() > 0) {
          out << "###marked nodes###\n";
          for(miter = marked_nodes.begin(); miter != marked_nodes.end(); miter++) {
            out << (*miter) << "\n";
          }
        }

        return out.str();
      }
        
      /** load the graph from a string 
       *  @param graph representing the graph
       */
      void load(std::string graph) {
        knowledge_nodes.clear();
        information_nodes.clear();
        data_nodes.clear();
        edges.clear();
        marked_nodes.clear();

        size_t pos = graph.find("\n");
        name = graph.substr(0, pos);
        graph = graph.substr(pos+1);

        pos = graph.find("\n");
        std::string knowledge = graph.substr(0, pos);
        graph = graph.substr(pos+1);
        while((pos = knowledge.find(";")) != std::string::npos) {
          KIDNode n;
          n.node = knowledge.substr(0, pos);
          knowledge_nodes.push_back(n);
          knowledge = knowledge.substr(pos+1);
        }

        pos = graph.find("\n");
        std::string information = graph.substr(0, pos);
        graph = graph.substr(pos+1);
        while((pos = information.find(";")) != std::string::npos) {
          KIDNode n;
          n.node = information.substr(0, pos);
          information_nodes.push_back(n);
          information = information.substr(pos+1);
        }

        pos = graph.find("\n");
        std::string data = graph.substr(0, pos);
        graph = graph.substr(pos+1);
        while((pos = data.find(";")) != std::string::npos) {
          KIDNode n;
          n.node = data.substr(0, pos);
          data_nodes.push_back(n);
          data = data.substr(pos+1);
        }

        pos = graph.find("###marked nodes###\n");
        std::string edges;
        if(pos != std::string::npos) {
          edges = graph.substr(0, pos);
          graph = graph.substr(pos+19);
        }
        else {
          edges = graph;
          graph = "";
        }

        while((pos = edges.find("\n")) != std::string::npos) {
          std::string edge = edges.substr(0, pos-1);
          size_t epos;
          KIDEdge e;
          epos = edge.find(";");
          e.from = edge.substr(0,epos);
          edge = edge.substr(epos+1);
          epos = edge.find(";");
          e.to = edge.substr(0,epos);
          edge = edge.substr(epos+1);
          epos = edge.find(";");
          e.label = edge.substr(0,epos);
          if(is_node(e.from) && is_node(e.to)) {
            this->edges.push_back(e);
            KIDNode *from = get_node(e.from);
            KIDNode *to   = get_node(e.to);
            from->children.push_back(to);
            to->parents.push_back(from);
          }
          edges = edges.substr(pos+1);
        }

        while((pos = graph.find("\n")) != std::string::npos) {
          std::string mark = graph.substr(0, pos);
          if(is_node(mark))
            marked_nodes.push_back(mark);
          graph = graph.substr(pos+1);
        }
      }
        
      /** checks if a node exists
       *  @param node name of the node to search for
       *  @return true if node exists, false otherwise
       */
      bool is_node(std::string node) {
        std::list<KIDGraph::KIDNode>::iterator iter;
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++)
          if(*iter == node)
            return true;
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++)
          if(*iter == node)
            return true;
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++)
          if(*iter == node)
            return true;

        return false;
      }

      /** checks if an edge exists
       *  @param edge edge to search for
       *  @return true if edge exists, false otherwise
       */
      bool is_edge(KIDEdge edge) {
        return (find(edges.begin(), edges.end(), edge) != edges.end());
      }

      /** gets the level of a node
       *  @param node node to get the level of
       *  @return level of the node {knowledge,information,data}
       */
      std::string level_of_node(std::string node) {
        if (find(knowledge_nodes.begin(), knowledge_nodes.end(), node) != knowledge_nodes.end())
          return "knowledge";
        else if (find(information_nodes.begin(), information_nodes.end(), node) != information_nodes.end())
          return "information";
        else if (find(data_nodes.begin(), data_nodes.end(), node) != data_nodes.end())
          return "data";
        else
          return "";
      }

      /** returns a pointer to a node specified by its name
       *  @param node name of the node
       *  @return pointer to the node or 0 if node does not exist
       */
      KIDNode* get_node(std::string node) {
        KIDNode* n = 0;
        std::list<KIDNode>::iterator iter;
        iter = find(knowledge_nodes.begin(), knowledge_nodes.end(), node);
        if(iter != knowledge_nodes.end()) {
          n=&(*iter);
          return n;
        }
        iter = find(information_nodes.begin(), information_nodes.end(), node);
        if(iter != information_nodes.end()) {
          n=&(*iter);
          return n;
        }
        iter = find(data_nodes.begin(), data_nodes.end(), node);
        if(iter != data_nodes.end()) {
          n=&(*iter);
          return n;
        }

        return n;
      }

      /** checks if a node is marked
       *  @param node name of the node to search for
       *  @return true if node is marked, false otherwise
       */
      bool is_marked(std::string node) {
        return (find(marked_nodes.begin(), marked_nodes.end(), node) != marked_nodes.end());
      }

      /** creates the dot representation of the graph
       *  @return string containing this graph in dot
       */
      std::string get_dot() {
        std::list<KIDGraph::KIDNode>::iterator iter;
        std::list<KIDGraph::KIDEdge>::iterator edgeiter;
        // standard "header" of the graph
        std::string dot = "digraph G {\n"\
                          "  ranksep=1;\n"\
                          "  edge[style=invis];\n"\
                          "  node[shape=box,fontsize=20,fixedsize=true,width=2];\n"\
                          "  \"Knowledge\" -> \"Information\" -> \"Data\";\n"\
                          "  edge[style=solid,dir=back];\n"\
                          "  node[shape=ellipse,fontsize=14,fixedsize=false,width=0.75];\n";
        
        // create knowledge nodes
        dot            += "  subgraph knowledge {\n"\
                          "    rank = same;\n"\
                          "    \"Knowledge\";\n";
        for(iter = knowledge_nodes.begin(); iter != knowledge_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red2]":"")+";\n";
        }
        dot            += "  }\n";

        // create information nodes
        dot            += "  subgraph information {\n"\
                          "    rank = same;\n"\
                          "    \"Information\";\n";
        for(iter = information_nodes.begin(); iter != information_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red2]":"")+";\n";
        }
        dot            += "  }\n";

        // create data nodes
        dot            += "  subgraph data {\n"\
                          "    rank = same;\n"\
                          "    \"Data\";\n";
        for(iter = data_nodes.begin(); iter != data_nodes.end(); iter++) {
          dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red2]":"")+";\n";
        }
        dot            += "  }\n";

        // create edges
        for(edgeiter = edges.begin(); edgeiter != edges.end(); edgeiter++) {
          dot          += "    \""+(edgeiter->to)+"\" -> \""+(edgeiter->from)+"\""+edgeiter->dot_edge(is_marked(edgeiter->from) && is_marked(edgeiter->to))+";\n";
        }

        // end the graph
        dot            += "}";
        
        return dot;
      }

      /** mark the node and it's children and parents
       *  @param node name of the node
       */
      void mark_node(std::string node) {
        //find nodes and edges connected to the markupnode
        if(node != "") {
          if(!is_marked(node))
            marked_nodes.push_back(node);

          //mark nodes pointing to marked nodes
          mark_children(get_node(node));

          //mark nodes pointed from marked nodes
          mark_parents(get_node(node));
        }
      }

      /** unmark the node and it's children and parents (if not otherwise marked)
       *  @param node name of the node
       */
      void unmark_node(std::string node) {
        if(node != "") {
          if(is_marked(node)) {
            marked_nodes.remove(node);
          }

          //unmark nodes pointed from marked nodes
          unmark_parents(get_node(node));

          //unmark nodes pointing to marked nodes
          unmark_children(get_node(node));
        }
      }

      /** clear the markup of the graph
       */
      void clear_markup() {
        marked_nodes.clear();
      }

    public:
      /** name of the graph */
      std::string             name;
      /** list of knowledge nodes */
      std::list<KIDNode>      knowledge_nodes;
      /** list of information nodes */
      std::list<KIDNode>      information_nodes;
      /** list of data nodes */
      std::list<KIDNode>      data_nodes;
      /** list of edges */
      std::list<KIDEdge>      edges;
    
    private:
      /** list of marked nodes */
      std::list<std::string>  marked_nodes;
  };
}

